
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <functional>
#include <limits>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include "tictactoe.h"

namespace mcts {

//******************************  Constants  ************************/

std::array<Cell, 3> C(const std::array<int, 3>& arr) {
    std::array<Cell, 3> ret { Cell(arr[0]), Cell(arr[1]), Cell(arr[2]) };

    return ret;
}

const std::array<std::array<Cell, 3>, 8> State::WIN_LINES = {  C({ 0, 1, 2 }), C({ 3, 4, 5 }), C({ 6, 7, 8 }),
     C({ 0, 3, 6 }), C({ 1, 4, 7 }), C({ 2, 5, 8 }), C({ 0, 4, 8 }), C({ 2, 4, 6 })  };

//******************************  Util functions  **********************/

/**
 * Moves are indexed starting at index 1.
 * So after MOVE_NONE = 0, MOVE(1) through MOVE(9) are
 * the moves with 'X' tokens, and MOVE(10) through MOVE(18) are those with 'O'.
 */
Token moveToToken(Move m)
{
    return Token(1+m/10);
}

Cell moveToCell(Move m)
{
    return Cell((m-1) % 9);
}

Move cellTokenToMove(Cell c, Token t)
{
    return Move( 1 + (int)c + (t-1) * 9 );
}

//*********************************  State  ******************************/

// Can do the TT queries with the id-keys, and put the rest of the data in a StateData object (which is fetched).
namespace Zobrist {

    std::array<Key, 19> ndx_keys { 0 };     // First entry will be 0, for the Empty initial state

    Key moveKey(Move move) {
        return ndx_keys[move];
    }

    Key tokenKey(Cell i, Token token) {
        //return ndx_keys[(1 + i) * (1 - ((token & 2) >> 1)) + ((token & 1) * 10)];
        return moveKey(cellTokenToMove(i, token));
    }

    Key cellKey(Cell i) {
        return tokenKey(i, X) ^ tokenKey(i, O);
    }

    Key terminalKey = 1;
    Key sideKey = 2;
    Key drawKey = 4;
    /**
     * The second bit tells us the next player to play.
     * the first bit tells us if state is terminal,
     * in which case the third bit tells us if it's a draw.
     *
     * 011 : 'X wins', 001 : 'O wins', 1*1 : 'draw', *00 : 'X to play', *10 : 'O to play'.
     */
    uint8_t status_key(const State& state) {
        Key ret = ((state.next_player()-1) << 1);
        ret ^= state.is_terminal();
        ret ^= (state.is_draw() << 2);

        return ret;
    }

    // TODO Implement the winner() method using WIN_LINES as bitmasks.
}  // namespace Zobrist


// TODO Try with smaller types (32bit) since the state space is so small.
void State::init()
{
     std::random_device rd;
     std::uniform_int_distribution<unsigned long long> dis(
         std::numeric_limits<std::uint64_t>::min(),
         std::numeric_limits<std::uint64_t>::max()
    );
    for (int i=1; i<19; ++i)
    {
        Zobrist::ndx_keys[i] = ((dis(rd) >> 3) << 3);    // Least three significant bits are reserved.
    }
}

State::State()
    : gamePly(1)
    , m_grid{}

{
    for (int i=0; i<9; ++i)
    {
        m_empty_cells.push_back(Cell(i));
    }
    data = new StateData();
    data->key = 0;
    data->gamePly = 1;
}

// State::State(grid_t&& grid)
//     : m_grid(std::move(grid))
// {
//     for (int i=0; i<9; ++i)
//     {
//         if (grid[i] == TOK_EMPTY)
//             m_empty_cells.push_back(Cell(i));
//     }
// }

Key State::key() const
{
    return data->key;
}

Token State::next_player() const
{
    return gamePly & 1 ? X : O;
    //return m_empty_cells.size() & 1 ? X : O;
}

bool State::is_full() const
{
    return gamePly > 9;
}

std::vector<Move>& State::valid_actions()
{
    m_valid_actions.clear();

    if (is_terminal())
    {
        return m_valid_actions;
    }

    auto token = next_player();
    for (auto c : m_empty_cells)
    {
        m_valid_actions.push_back(cellTokenToMove(c, token));
    }
    return m_valid_actions;
}

Token State::winner() const
{
    for (const auto& row : WIN_LINES) {
        auto [a, b, c] = row;

        if (1 == (m_grid[a] * m_grid[b] * m_grid[c])) { return X; }
        if (6 == (m_grid[a] + m_grid[b] + m_grid[c])) { return O; }
    }
    return TOK_EMPTY;
}

bool State::is_terminal() const
{
    return is_full() || winner() != TOK_EMPTY;
}

bool State::is_draw() const
{
    return is_full() && winner() == TOK_EMPTY;
}

void State::apply_move(Move m, StateData& new_sd)
{
    Key key = data->key;
    // Copy the needed StateData fields to the new one,
    // then replace it with the new one.
    new_sd.gamePly = gamePly + 1;
    new_sd.previous = data;
    data = &(new_sd);

    // Make sure the move corresponds to an empty cell
    auto cell = moveToCell(m);
    assert(m_empty_cells.remove(cell) == 1);

    // Place the new token in the cell
    m_grid[(int)cell] = moveToToken(m);
    ++gamePly;

    // Update the key for the move
    key ^= Zobrist::moveKey(m);

    // Update the key for the game status
    key ^= Zobrist::sideKey;
    key ^= is_terminal();               // Indicate winner with second bit (next to play).
    key ^= (is_draw() << 2);            // Indicate draw with first and third bit.

    // Place the key in the new StateData.
    data->key = key;
}

// TODO Save the data discarded somehwere!
// TODO Just save the previous move in the stack of StateData,
// then use that move to call undo_move (as it is, there is no
// guarantee that the previous StateData corresponds to the state
// we're reverting back to)
void State::undo_move(Move m)
{
    // Make sure the move corresponds to a non-empty cell.
    auto cell = moveToCell(m);
    assert(m_grid[(int)cell] != TOK_EMPTY);

    // Remove the token from the grid.
    m_grid[(int)cell] = TOK_EMPTY;
    m_empty_cells.push_back(cell);

    // Revert the StateData.
    --gamePly;
    data = data->previous;
}

const State::grid_t& State::grid() const
{
    return m_grid;
}

const std::list<Cell>& State::empty_cells() const
{
    return m_empty_cells;
}

} // namespace mcts
