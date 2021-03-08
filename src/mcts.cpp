#include <iostream>
#include <assert.h>
#include <math.h>
#include "mcts.h"

namespace mcts {

MCTSLookupTable MCTS;

//****************************** Utility functions ***********************/

// get_node queries the Hash Table until it finds the position,
// creating a new entry in case it doesn't find it.
Node* get_node(const State& state)
{
    Key state_key = state.key();

    auto node_it = MCTS.find(state_key);
    if (node_it != MCTS.end())
    {
        return &(node_it->second);
    }

    // Insert the new node in the Hash table if it wasn't found.
    Node new_node;
    new_node.key                    = state_key;

    auto new_node_it = MCTS.insert(std::make_pair(state_key, new_node)).first;
    return &(new_node_it->second);
}

//******************************* Time management *************************/
using TimePoint = std::chrono::milliseconds::rep;

std::chrono::time_point<std::chrono::steady_clock> search_start;

void init_time()
{
    search_start = std::chrono::steady_clock::now();
}

TimePoint time_elapsed()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - search_start).count();
}

//******************************** Ctor(s) *******************************/

Agent::Agent(State& state)
    : state(state)
    , nodes{}
{
    create_root();
}

//******************************** Main methods ***************************/

Move Agent::MCTSBestMove()
{
    init_time();
    create_root();

    while (computation_resources())
    {
        Node* node = tree_policy();
        Reward reward = rollout_policy(node);
        backpropagate(node, reward);
    }

    return best_visits(root)->move;
}

void Agent::create_root()
{
    ply = 1;
    iteration_cnt = 0;

    // Reset the buffers
    nodes = {};

    root = nodes[ply] = get_node(state);

    if (root->n_visits == 0)
    {
        init_children();
        ++root->n_visits;
    }
}

bool Agent::computation_resources()
{
    return time_elapsed() > MAX_TIME - 100 || iteration_cnt > MAX_ITER;
}

Node* Agent::tree_policy()
{
    assert(current_node() == root);

    if (root->n_children == 0)
        return root;

    while (current_node()->n_visits > 0)
    {
        if (is_terminal(current_node()))
            return current_node();

        actions[ply] = best_uct(current_node());
        Move move = actions[ply]->move;

        ++current_node()->n_visits;

        apply_move(move);
        nodes[ply] = get_node(state);    // Before breaking, this will create a new node in the MCTS lookup table.
    }

    return current_node();
}

Reward Agent::rollout_policy(Node* node)
{
    assert(node == current_node());
    assert(node->n_visits == 0);

    if (is_terminal(node))
    {
        return evaluate_terminal();
    }

    init_children();                      // Expand the node and do a rollout on each child, return max reward.

    return node->children[0].prior_value;
}

// Note: as in Stockfish's, we could backpropagate minimax of avg_value instead of rollout reward.
// (the more confident we are in our sampling, the more we want to propagate extremal results only?)
void Agent::backpropagate(Node* node, Reward r)
{
    assert(node == current_node());

    while (current_node() != root)
    {
        undo_move();
        r = 1.0 - r;    // Undoing move changes player.

        ActionNode* action = actions[ply];

        ++action->n_visits;

        action->action_value += r;
        action->avg_action_value = action->action_value / action->n_visits;

        // Adjust/change r here as wanted.
    }
}

ActionNode* Agent::best_uct(Node* node)
{
    int best = -1;
    auto best_val = -std::numeric_limits<double>::max();

    for (int i=0; i<node->n_children; ++i)
    {
        auto r = node->children[i].avg_action_value + exploration_cst * sqrt( log(node->n_visits) / node->children[i].n_visits+1 );
        if (r > best_val)
        {
            best_val = r;
            best = i;
        }
    }

    return &(node->children[best]);
}

ActionNode* Agent::best_visits(Node* node)
{
    int best = -1;
    auto best_val = -std::numeric_limits<int>::max();

    for (int i=0; i<node->n_children; ++i)
    {
        auto v = node->children[i].n_visits;
        if (v > best_val)
        {
            best_val = v;
            best = i;
        }
    }

    return &(node->children[best]);
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

        if (current_node()->n_children < MAX_CHILDREN)
        {
            *current_node()->end() = new_action;
            ++current_node()->n_children;
        }
    }

    std::sort(current_node()->begin(), current_node()->end(), [](const auto& a, const auto& b){
            return a.prior_value < b.prior_value;
        });

}

void Agent::apply_move(Move move)
{
    ++ply;
    state.apply_move(move);
}

void Agent::undo_move()
{
    --ply;
    state.undo_move(actions[ply]->move);
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
    Token token = current_node()->last_move < 10 ? X : O;
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
