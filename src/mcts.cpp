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

Reward Agent::rollout_policy(Node* node)
{

}


//***************************** Playing moves ******************************/

Node* Agent::current_node()
{
    return nodes[ply];
}

bool Agent::is_terminal()
{
    return false;
}

void Agent::init_children()
{
    auto& valid_actions = state.valid_actions();
    //auto& children      = current_node()->children_list();

    for (auto move : valid_actions)
    {
        Reward prior = calculate_prior(move);

        ActionNode new_action;
        new_action.move = move;
        new_action.n_visits = 0;
        new_action.prior_value = prior;
        new_action.action_value = 0;
        new_action.avg_action_value = 0;
    }
}

//***************************** Evaluation of nodes **************************/

Reward Agent::random_simulation(Move move)
{
    Node* node = current_node();
    return 0.5;
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
