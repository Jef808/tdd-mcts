#ifndef __NODE_H_
#define __NODE_H_

#include <memory>
#include <optional>
#include <vector>
#include <ranges>
#include <range/v3/view.hpp>
#include <range/v3/algorithm/any_of.hpp>

namespace mcts {

template<class S>
using Move = typename S::move_t;


template <size_t N>
class Zobrist {
};

template<class S>
struct NodeAction {
    using action_t = typename S::action_t;

    action_t m_action;
    bool m_expanded;
    void mark_expanded() { m_expanded = true; }
    bool expanded() const { return m_expanded; }

    NodeAction(action_t a, bool e = false)
        : m_action(a)
        , m_expanded(e)
    {
    }
    bool operator==(const NodeAction& other) const {
        return m_action == other.m_action;
    }
};

/**
 * Eventually, we need to do the following: encode the states with a Zobrist hash
 * and only keep a list of valid actions.
 * They are stored as their 'Zobrist keys' and we decorate them with a bool to mark
 * whether or not they've been used yet.
 *
 * The drawback is we have to reimplement some of the logic of the game in functors
 * later on. For now, let's store a copy of the whole state.
 */
template <class S>
class NodeState {
public:
    typedef NodeAction<S> naction_t;
    using a_cont_t = std::vector<naction_t>;

    NodeState(const S& state)
        : m_state(state)
        , m_valid_actions(to_naction_t(state.valid_actions()))
    {
    }
    const a_cont_t& valid_actions() const { return m_valid_actions; }
    a_cont_t unexpanded_actions() const;
    NodeState expand_action(int action_ndx) const
    {
        valid_actions()[action_ndx].mark_expanded();
        return NodeState(m_state);
    }

private:
    S m_state;
    std::vector<naction_t> m_valid_actions;
    int n_visits;
    float m_value;

    std::vector<naction_t> to_naction_t(std::vector<typename S::action_t> actions) const
    {
        return actions | ranges::views::transform([](const auto& a){return naction_t{a};})
                       | ranges::to<std::vector>;
    }

    S state_from_action(naction_t& action) const
    {
        S nex_state = action(m_state);
        action.mark_expanded();
        return nex_state;
    }
};

template<class S>
std::vector<typename NodeState<S>::naction_t> NodeState<S>::unexpanded_actions() const
{
    auto not_expanded = [](const auto& a){ return !a.expanded(); };
    return valid_actions()
        | ranges::views::filter(not_expanded)
        | ranges::to<std::vector>;
}


} // namespace mcts

#endif // __NODE_H_