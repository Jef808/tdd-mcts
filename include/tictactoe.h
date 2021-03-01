#ifndef __TICTACTOE_H_
#define __TICTACTOE_H_

#include <array>
#include <optional>
#include <vector>

namespace ttt {

    /** Represents the possible values for the state's cells. */
    enum class Token { X, O }; //, EMPTY };

    /** Class that implements the changes that can be done to a state. */
    class Action;

    /**
     * State class, holds the data of the game and manages the various rules.
     * Allows Action as a friend class so that none of State's methods have
     * any side effect.
    */
    class State {

        using row_t = std::array<int, 3>;
        static const std::array<row_t, 8> WIN_COMBIN;

        public:
            using ndx_t = int;
            using value_t = std::optional<Token>;
            using grid_t = std::array<value_t, 9>;

            /** Initialization of a state from a grid. */
            State(grid_t&& = grid_t());
            ///** Deleted copy constructor. */
            //State(const State&) = delete;
            /** Determine if the state represents a game that's over. */
            bool is_terminal() const;
            const grid_t& grid() const;
        private:
            friend class Action;

            /** The internal data of the state. */
            grid_t  m_grid;
    };

    /**
     * Changes the token at a specified index to a specified token.
     * Actions with `m_token' equal to `Token::EMPTY' do nothing.
    */
    class Action {
        using ndx_t = State::ndx_t;
        using value_t = State::value_t;
        using grid_t = State::grid_t;

        public:
            Action(ndx_t, value_t);
            /** Applies this action on a state in place. */
            State& operator()(State&) const;
            /** Applies this action on a copy of the state */
            State operator()(const State&) const;

        private:
            ndx_t m_ndx;
            value_t m_val;

            /** Implementation of the action. */
            grid_t& apply_to(grid_t&) const;

    };
    
} // ttt


#endif // __TICTACTOE_H_
