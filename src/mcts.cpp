#include <iostream>
#include <assert.h>
#include "mcts.h"

namespace mcts {

MCTSLookupTable MCTS;

//****************************** Utility functions ***********************/

// get_node queries the Hash Table until it finds the position,
// creating a new entry in case it doesn't find it.
Node* get_node(const State& state)
{
    Key state_key = state.get_key();

    auto node_it = MCTS.find(state_key);
    if (node_it != MCTS.end())
    {
        return &(node_it->second);
    }

    // Insert the new node in the Hash table if it wasn't found.
    Node new_node;
    new_node.key                    = state_key;

    auto new_node_it = MCTS.insert(std::make_pair(state_key, new_node));
    return &(new_node_it.first->second);
}

Move select_random(const std::vector<Move>& moves)
{
    return moves[rand() % moves.size()];
}

//******************************** Ctor(s) *******************************/

Agent::Agent(State& state)
    : state(state)
{
    nodes.reserve(MAX_PLY);

    create_root();
}

//******************************** Main methods ***************************/

void Agent::create_root()
{
    ply = 0;
    iteration_cnt = 0;

    // Reset the buffer
    nodes.clear();

    root = get_node(state);
    nodes.push_back(root);

    if (root->n_visits == 0)
    {
        init_children();
    }
}

Node* Agent::tree_policy()
{
    assert(current_node() == root);

}

Reward Agent::rollout_policy(Node* node)
{
    assert(node == current_node());
    assert(node->n_visits == 0);

    if (is_terminal(node))
    {
        return evaluate_terminal();
    }

    init_children();
    ++node->n_visits;

    return node->children_list().begin()->prior_value;
}


//***************************** Playing moves ******************************/

Node* Agent::current_node()
{
    return nodes[ply];
}

bool Agent::is_terminal(Node* node)
{
    return false;
}

void Agent::init_children()
{
    auto& valid_actions = state.valid_actions();
    //auto& children      = current_node()->children_list();

    for (auto move : valid_actions)
    {
        Reward prior = random_simulation(move);

        ActionNode new_action;
        new_action.move = move;
        new_action.n_visits = 0;
        new_action.prior_value = prior;
        new_action.action_value = 0;
        new_action.avg_action_value = 0;

        current_node()->children_list().push_back(new_action);
    }

    std::sort(current_node()->begin(), current_node()->end(), [](const auto& a, const auto& b){
            return a.prior_value < b.prior_value;
        });
}

//***************************** Evaluation of nodes **************************/

Reward Agent::random_simulation(Move move)
{
    State sim_state = state.clone();
    sim_state.apply_move(move);
    std::vector<Move> valid_moves;

    while (!sim_state.is_terminal())
    {
        valid_moves = sim_state.valid_actions();
        sim_state.apply_move(valid_moves[rand() % valid_moves.size()]);
    }

    Token token = (int)move < 10 ? X : O;
    Token winner = sim_state.winner();
    if (winner == token)
    {
        return 1.0;
    }
    else if (winner == TOK_EMPTY)
    {
        return 0.5;
    }
    return 0.0;
}

Reward Agent::evaluate_terminal()
{
    Token token = (int)current_node()->last_move < 10 ? X : O;
    Token winner = state.winner();
    if (winner == token)
    {
        return 1.0;
    }
    else if (winner == TOK_EMPTY)
    {
        return 0.5;
    }
    return 0.0;
}
// Node* Agent::selection_policy()
// {
//     assert(current_node() == root);
//     return current_node();
// }

// void Agent::init_actionchild(Node* node, Move move, Reward rollout_score, int move_cnt)
// {


// }
} // namespace mcts
