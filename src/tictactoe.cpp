#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include "tictactoe.h"

namespace ttt {

//******************************  Constants  ************************/

//const grid_t EMPTY_GRID = {Token::EMPTY};

const std::array<State::row_t, 8> State::WIN_COMBIN = {{ { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 },
            { 0, 3, 6 }, { 1, 4, 7 }, { 2, 5, 8 }, { 0, 4, 8 }, { 2, 4, 6 } }};


//*********************************  State  ******************************/

    State::State(grid_t&& grid)
        : m_grid(std::move(grid))
    {
    }

    // TODO implement `is_terminal`

    const State::grid_t& State::grid() const
    {
        return m_grid;
    }

//*******************************  Action  ****************************/

    Action::Action(ndx_t ndx, value_t val)
        : m_ndx(ndx)
        , m_val(val)
    {
    }

    Action::grid_t& Action::apply_to(grid_t& grid) const
    {
        grid[m_ndx] = m_val;
        return grid;
    }

    State& Action::operator()(State& state) const
    {
        this->apply_to(state.m_grid);
        return state;
    }

    State Action::operator()(const State& state) const
    {
        auto grid_cpy = grid_t(state.grid());
        this->apply_to(grid_cpy);
        return State(std::move(grid_cpy));
    }


} // ttt
