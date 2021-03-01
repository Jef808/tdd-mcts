#include "tictactoe.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

namespace ttt {

//******************************  Constants  ************************/

const std::array<std::array<int, 3>, 8> State::WIN_COMBIN = { { { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 },
    { 0, 3, 6 }, { 1, 4, 7 }, { 2, 5, 8 }, { 0, 4, 8 }, { 2, 4, 6 } } };

//*********************************  State  ******************************/

State::State()
    : m_grid()
{
}
State::State(grid_t&& grid)
    : m_grid(std::move(grid))
{
}
State::State(const grid_t& grid)
    : m_grid(grid)
{
}

const State::grid_t& State::grid() const
{
    return m_grid;
}

bool State::is_full() const
{
    bool res = true;
    ;
    for (auto& val : m_grid) {
        if (!val.has_value()) {
            res = false;
            break;
        }
    }
    return res;
}

bool State::has_winner() const
{
    for (const auto& row : WIN_COMBIN) {
        auto val = m_grid[row[0]];
        if (val.has_value() && m_grid[row[1]] == val && m_grid[row[2]] == val) {
            return true;
        }
    }
    return false;
}

bool State::is_terminal() const
{
    return is_full() || has_winner();
}

ttt::Token State::next_player() const
{
    auto cells_filled = std::count_if(begin(m_grid), end(m_grid), [](const auto& a) {
        return a.has_value();
    });

    return cells_filled & 1 ? Token::O : Token::X;
}

std::vector<Action> State::valid_actions() const
{
    Token next_token = next_player();
    std::vector<Action> res;
    for (int i = 0; i < 9; ++i) {
        if (!m_grid[i].has_value()) {
            res.push_back(Action(i, next_token));
        }
    }
    return res;
}

//*******************************  Action  ****************************/

Action::Action(ndx_t ndx, value_t val)
    : m_ndx(ndx)
    , m_val(val)
{
}

Action::grid_t& Action::apply_to(grid_t& grid) const
{
    auto old_val = grid[m_ndx];
    if (!old_val.has_value())
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

Action::ndx_t Action::ndx() const
{
    return m_ndx;
}
Action::value_t Action::val() const
{
    return m_val;
}

bool Action::operator==(const Action& other) const
{
    return m_ndx == other.m_ndx && m_val == other.m_val;
}

} // ttt
