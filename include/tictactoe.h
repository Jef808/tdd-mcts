#ifndef __TICTACTOE_H_
#define __TICTACTOE_H_

#include <array>
#include <optional>
#include <vector>

namespace ttt {

/** Represents the possible values for the state's cells. */
enum class Token { X,
    O }; //, EMPTY };

/** Class that implements the changes that can be done to a state. */
class Action;

/**
 * State class, holds the data of the game and manages the various rules.
 * Allows Action as a friend class so that none of State's methods have
 * any side effect.
*/
class State {
public:
    using ndx_t = int;
    using value_t = std::optional<Token>;
    using grid_t = std::array<value_t, 9>;

    State();
    explicit State(const grid_t&);
    explicit State(grid_t&&);
    bool is_terminal() const;
    std::vector<Action> valid_actions() const;
    const grid_t& grid() const;

private:
    friend class Action;
    /** The internal data of the state. */
    grid_t m_grid;
    bool is_full() const;
    bool has_winner() const;
    Token next_player() const;
    static const std::array<std::array<ndx_t, 3>, 8> WIN_COMBIN;
};

/**
 * Changes the token at a specified index to a specified token.
 * Actions with `m_token' equal to `Token::EMPTY' do nothing.
*/
class Action {
public:
    using ndx_t = State::ndx_t;
    using value_t = State::value_t;
    using grid_t = State::grid_t;

    Action(ndx_t, value_t);
    ndx_t ndx() const;
    value_t val() const;
    State& operator()(State&) const;
    State operator()(const State&) const;
    bool operator==(const Action&) const;

private:
    ndx_t m_ndx;
    value_t m_val;

    /** Implementation of the action. */
    grid_t& apply_to(grid_t&) const;
};

} // ttt

#endif // __TICTACTOE_H_
