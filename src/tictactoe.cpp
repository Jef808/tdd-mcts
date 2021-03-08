
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
        std::array<Cell, 3> ret { (Cell)(arr[0]), (Cell)(arr[1]), (Cell)(arr[2]) };

        return ret;
    }

const std::array<std::array<Cell, 3>, 8> State::WIN_LINES = {  C({ 0, 1, 2 }), C({ 3, 4, 5 }), C({ 6, 7, 8 }),
     C({ 0, 3, 6 }), C({ 1, 4, 7 }), C({ 2, 5, 8 }), C({ 0, 4, 8 }), C({ 2, 4, 6 })  };

//******************************  Util functions  **********************/

Token moveToToken(Move m)
{
    return Token(1+m/10);
}

Cell moveToCell(Move m)
{
    return Cell(m % 10);
}

Move cellTokenToMove(Cell c, Token t)
{
    return Move( (int)c + (t-1) * 10 );
}

//*********************************  State  ******************************/

namespace Zobrist {

    std::array<Key, 19> ndx_keys { 0 };         // First entry is 0, for the Empty initial state

    /**
     * So move m corresponds to (Cell, Token) = (m % 10 + 1, (int)(m / 10.0))
     * MOVE_NULL = 0 corresponds to the empty state, then MOVE(1) through MOVE(9)
     * are the moves with 'X' tokens, and MOVE(10) through MOVE(18) are those with 'O'.
     */
    Key moveKey(Move move) {
        return ndx_keys[move];
    }

    Key tokenKey(Cell i, Token token) {
        return ndx_keys[(1 + i) * (1 - ((token & 2) >> 1)) + ((token & 1) * 10)];
    }

    Move tokenToMove(Cell i, Token token) {
        return (Move)(1 + (int)i + token == X ? 1 : 11);      // (i, 'X') -> i+1 , (i, 'O') -> i+1 + 10 , (i, TOK_EMPTY) -> 0
    }

    /**
     * The second bit tells us the next player to play.
     * the second bit tells us the winner (0 for X, 1 for O),
     * while if it's a draw, the third bit is toggled on.
     *
     * 001 : 'X wins', 011 : 'O wins', 1*1 : 'draw', *00 : 'X to play', *10 : 'O to play'.
     */
    Key status_key(const State& state) {

        if (!state.is_terminal())
        {
            return state.next_player() << 1;
        }
        return state.winner() << 1 ^ 1;
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
    : m_grid()
    , m_empty_cells()
    , m_valid_actions()
{
    for (int i=1; i<10; ++i)
    {
        m_empty_cells.push_back(Cell(i));
    }
}

State::State(grid_t&& grid)
    : m_grid(std::move(grid))
    , m_empty_cells()
    , m_valid_actions()
{
    for (int i=1; i<10; ++i)
    {
        if (grid[i-1] == TOK_EMPTY)
            m_empty_cells.push_back(Cell(i));
    }
}
// State::State(const grid_t& grid)
//     : m_grid(grid)
// {
// }

Key State::get_key() const
{
    Key res = 0;
    const auto begin = m_grid.begin();
    auto it = begin;
    while (it != m_grid.end())
    {
        auto ndx = std::distance(begin, it);
        res ^= Zobrist::tokenKey((Cell)ndx, *it) ;
        ++it;
    }
    res ^= Zobrist::status_key(*this);
    return res;
}

Key State::apply_move(Move move, Key key) const
{
    return key ^ Zobrist::moveKey(move);
}

bool State::is_terminal(Key key)
{
    return key & 1;
}

Token State::next_player(Key key) const
{
    return (Token)(key >> 1 & 1);
}

Token State::winner(Key key)
{
    return (Token)((key & 6) >> 1);
}


Token State::next_player() const
{
    // auto cells_filled = std::count_if(begin(m_grid), end(m_grid), [](const auto& a) {
    //     return a != TOK_EMPTY;
    // });

    return m_empty_cells.size() & 1 ? X : O;
}

/**
 * Lighter version of valid_actions() used to check if
 * state is terminal
 */
bool State::full() const
{
    // a >> 1 is one if and only if a is empty
    // std::any_of(begin(m_grid), end(m_grid), [](auto a){ return a >> 1 & 1; });
    return m_empty_cells.empty();
}

std::vector<Move>& State::valid_actions()
{
    m_valid_actions.clear();

    auto token = next_player();
    for (auto c : m_empty_cells)
    {
        m_valid_actions.push_back(cellTokenToMove(c, token));
    }

    // auto cbegin = m_grid.cbegin();
    // auto it = m_grid.begin();
    // size_t cnt = 0;

    // while (it != m_grid.end() && *it != TOK_EMPTY)
    // {
    //     size_t ndx = std::distance(cbegin, it);
    //     Move mv = Move(ndx + (*it == X ? 1 : 10));
    //     m_valid_actions[cnt] = mv;
    //     ++cnt;
    //     ++it;
    // }
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
    return full() || winner() != TOK_EMPTY;
}

State& State::apply_move(Move m)
{
    auto cell = moveToCell(m);
    assert(m_empty_cells.remove(cell) == 1);

    m_grid[(int)cell - 1] = moveToToken(m);

    return *this;
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
