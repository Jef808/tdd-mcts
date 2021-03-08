#ifndef __TICTACTOE_H_
#define __TICTACTOE_H_

#include <array>
#include <list>
#include <bits/stdint-uintn.h>
#include <vector>

namespace mcts {

// TODO Update the Keys methods. I change TOK_EMPTY from 2 to 0 so that arrays initialize to
// the correct token by default.
enum Token {
    TOK_EMPTY,
    X,
    O,
};

enum Cell : int {
    CELL_NONE = -1,
    CELL_END = 9
};

enum Move : int {
    MOVE_NONE,
    MOVE_END = 19
};

typedef uint64_t Key;

/**
 * From a StateData object, the state can be reconstructed, or
 * a move can be undone.
 */
struct StateData {
    Key key;
    int gamePly;
    StateData* previous;
};

class State {
    public:
    using grid_t = std::array<Token, 9>;

    static void init();

    State();
    // explicit State(const grid_t&);
    explicit State(grid_t&&);  // Only for testing.
    State clone() const;
    // Game logic
    Token winner() const;
    Token next_player() const;
    bool is_full() const;
    bool is_terminal() const;
    bool is_draw() const;
    std::vector<Move>& valid_actions();
    State& apply_move(Move);
    void apply_move(Move, StateData&);
    void undo_move(Move);

    // Zobrist keys
    Key key() const;

    // Game logic encoded in bitstrings.
    bool is_terminal(Key);
    Token winner(Key);
    Token next_player(Key);
    //static Key after_move(Key, Move);

    const grid_t& grid() const;                 // Only for testing.
    const std::list<Cell>& empty_cells() const; // Only for testing

private:
    static const std::array<std::array<enum Cell, 3>, 8> WIN_LINES;
    grid_t m_grid;
    StateData* data;
    std::list<Cell> m_empty_cells;
    std::vector<Move> m_valid_actions;

    int gamePly = 1;
};

} // namespace mcts


#endif // __TICTACTOE_H_
