#ifndef __TICTACTOE_H_
#define __TICTACTOE_H_

#include <array>
#include <list>
#include <vector>
#include "type.h"

namespace mcts {

// TODO Update the Keys methods. I change TOK_EMPTY from 2 to 0 so that arrays initialize to
// the correct token by default.

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
    // explicit State(grid_t&&);
    // Game logic
    Token winner() const;
    Token next_player() const;
    bool is_full() const;
    bool is_terminal() const;
    bool is_draw() const;
    bool is_valid(Move move) const;
    std::vector<Move>& valid_actions();
    void apply_move(Move, StateData&);
    void undo_move(Move);

    // Zobrist keys
    Key key() const;

    StateData* data;
    int gamePly = 1;

    const grid_t& grid() const;                 // Only for testing.
    const std::list<Cell>& empty_cells() const; // Only for testing

    static Token moveToToken(Move m);
    static Cell moveToCell(Move m);
    static Move cellTokenToMove(Cell c, Token t);

private:
    static const std::array<std::array<enum Cell, 3>, 8> WIN_LINES;
    grid_t m_grid;
    std::list<Cell> m_empty_cells;
    std::vector<Move> m_valid_actions;


};

inline bool key_terminal(Key key) {
    return key & 1;
}

inline Token key_next_player(Key key) {
    return Token(1 + (key >> 1 & 1));
}

inline Token key_winner(Key key) {
    return (key >> 2) & 1 ? TOK_EMPTY                  // 1*1
                          : (key >> 1) & 1 ? X         // *11
                                           : O;        // *01
}

inline double key_ev_terminal(StateData sd) {
    return (sd.key >> 2) & 1 ? 0.5    // From the pov of the player who just played,
                             : 1;     // either it is a draw or a win (reflect token in backpropagation)
}

} // namespace mcts


#endif // __TICTACTOE_H_
