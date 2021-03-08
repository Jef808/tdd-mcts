#ifndef __TICTACTOE_H_
#define __TICTACTOE_H_

#include <array>
#include <list>
#include <bits/stdint-uintn.h>
#include <vector>

namespace mcts {

// typedef struct Ndx {
//     enum _Ndx : int {
//         _BEGIN = 0,
//         _NULL = 10
//     } m_ndx;
//     Ndx operator++() {
//         int tm_ndx = m_ndx;
//         if (++tm_ndx < 10) { return (_Ndx)(tm_ndx); } { return _BEGIN; }
//     }
//     operator int() { return (int)m_ndx; }
//     Ndx(int i) : m_ndx((_Ndx)i) { }
// } Ndx;

// TODO Update the Keys methods. I change TOK_EMPTY from 2 to 0 so that arrays initialize to
// the correct token by default.
enum Token {
    TOK_EMPTY,
    X,
    O,
};

enum Cell {
    CELL_NULL,
    CELL_END = 10
};

enum Move : int {
    MOVE_NULL,
    MOVE_END=20
};

typedef uint64_t Key;

/**
 * From a StateData object, the state can be reconstructed, or
 * a move can be undone.
 */
struct StateData {
    Key key;
    StateData* previous;
};

class State {
    public:
    using grid_t = std::array<Token, 9>;

    static void init();

    State();
    // explicit State(const grid_t&);
    explicit State(grid_t&&);

    // Game logic
    Token winner() const;
    Token next_player() const;
    bool full() const;
    bool is_terminal() const;
    std::vector<Move>& valid_actions();
    State& apply_move(Move);
    State& undo_move(Move);

    // Zobrist keys
    Key get_key() const;

    // Game logic encoded in bitstrings.
    static bool is_terminal(Key);
    static Token winner(Key);
    Token next_player(Key) const;
    Key apply_move(Move, Key) const;
    Key undo_move(Move, Key) const;

    const grid_t& grid() const;                 // Only for testing.
    const std::list<Cell>& empty_cells() const; // Only for testing

private:
    static const std::array<std::array<enum Cell, 3>, 8> WIN_LINES;
    grid_t m_grid;
    StateData* data;
    std::list<Cell> m_empty_cells;
    std::vector<Move> m_valid_actions;
};

} // namespace mcts


#endif // __TICTACTOE_H_
