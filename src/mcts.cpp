#include <iostream>
#include <assert.h>
#include <math.h>
#include "mcts.h"
#include "debug.h"


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
    //new_node.last_move              = actions[ply]->move;

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
    , stackBuf{}
{
    create_root();
}

//******************************** Main methods ***************************/

Move Agent::MCTSBestMove()
{
    init_time();
    create_root();

    int init_ply = ply;

    while (computation_resources())
    {
        Node* node = tree_policy();

        // TODO Maybe need to return best_avg_value here since we know all the tree.
        if (node->n_visits > 0)
        {
            while (ply > init_ply)
                undo_move();
            break;
        }

        Reward reward = rollout_policy(node);
        backpropagate(node, reward);
        ++iteration_cnt;
    }

    if (debug_counters)
    {
        std::cerr << "Iterations: " << iteration_cnt << '\n';
        std::cerr << "Descent count: " << descent_cnt << '\n';
        std::cerr << "Rollout count: " << rollout_cnt << '\n';
        std::cerr << "Exploration count: " << explored_nodes_cnt << '\n';
        std::cerr << "Number of nodes in table: " << MCTS.size() << std::endl;
    }

    auto* choice = best_visits(root);

    if (debug_main_methods)
        std::cerr << "returning from main method" << std::endl;

    return choice->move;
}

void Agent::create_root()
{
    ply                 = state.data->gamePly;
    states[ply].gamePly = ply;
    states[ply].key     = state.key();

    // Reset the buffers
    nodes = {};

    // Reset the counters
    iteration_cnt       = 0;
    rollout_cnt         = 0;
    descent_cnt         = 0;
    explored_nodes_cnt  = 0;

    root = nodes[ply] = get_node(state);

    if (root->n_visits == 0)
    {
        init_children();
    }
}

bool Agent::computation_resources()
{
    // bool time_ok = time_elapsed() < MAX_TIME - 100;    // Clock keeps running when using gdb.
    bool iteration_cnt_ok =  iteration_cnt < MAX_ITER;

    return iteration_cnt_ok;
}

Node* Agent::tree_policy()
{
    assert(current_node() == root);

    if (debug_main_methods)
        std::cerr << "Initializing descent " << descent_cnt << std::endl;

    if (root->n_children == 0)
        return root;

    ++descent_cnt;

    // At each previously explored node, choose an action in the direction that
    // is "most important" to sample in the tree. (i.e. minimizing regret in long
    // or short term)
    while (current_node()->n_visits > 0)
    {
        if (is_terminal(current_node()))
            return current_node();

        // Keep record of number of times each part of the tree has been sampled.
        ++current_node()->n_visits;

        // The choice of Edge (action) at each node is driven by the uct policy
        actions[ply] = best_uct(current_node());

        Move move = actions[ply]->move;  // Keep record of the path we're tracing to go back along it.
        assert(move != MOVE_NONE);       // TODO Remove this
        apply_move(move);

        nodes[ply] = get_node(state);    // Either the node has been seen and is associated to a state key,
                                         // or not and get_node creates a record of it.

        // FIXME How to deal with different paths (different nodes in the tree) leading
        // to equivalent state position? The state key will be the same but not the nodes.
        // Have to make sure to only keep the best path and reorganize the tree without losing information.
        // NOTE in this direction : currently the last_move field of a Node object isn't used! it can have
        // multiple parents. This most likely messes with the algorithm by favoring positions that arise
        // more frequenty.
    }

    // At this point, we reached an unexplored node and it is time to explore the game tree from
    // there.
    if (debug_main_methods)
    {
        std::cerr << "Chosen node \n";
        Debug::display(std::cerr, state);
        std::cerr << std::endl;
    }

    return current_node();
}

// Once we have the next unexplored node, we do a random
// playout starting with every one of its children.
Reward Agent::rollout_policy(Node* node)
{
    if (debug_main_methods)
    {
        std::cerr << "Rollout on node with state \n";
        Debug::display(std::cerr, state);
    }

    assert(node == current_node());
    assert(node->n_visits == 0);

    // NOTE: this doesn't seem to be necessary, we do
    // the same check when doing the random_simulation. Well, it would be
    // node->children will be { MOVE_END } and we would need to make a check before return
    // the prior_value. That would all be a waste of function calls for nothing.
    //
    if (is_terminal(node))
    {
        ++node->n_visits;    // "Exploring node"
        return 1 - evaluate_terminal();
    }

    init_children();                      // Expand the node and do a rollout on each child, return max reward.

    return node->children.front().prior_value;
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

Reward Agent::evaluate_terminal()
{
    return key_ev_terminal(states[ply]);
}

ActionNode* Agent::best_uct(Node* node)
{
    auto best = -1;
    auto best_val = -std::numeric_limits<double>::max();

    // TODO update for list container.
    for (int i=0; i<node->n_children; ++i)
    {
        auto* c = &(node->children[i]);

        if (c->n_visits < 1)
        {
            return c;    // children are already ordered by à priori value in this case.n
        }

        Reward r = (double)c->avg_action_value + (double)exploration_cst * sqrt( (double)(log(node->n_visits)) / (double)(c->n_visits+1) );
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
    if (debug_best_visits)
        std::cerr << "Choosing best visits. choices are :";

    int best = 0;
    auto best_val = -std::numeric_limits<int>::max();

    auto& children = node->children_list();

    for (int i=0; i<node->n_children; ++i)
    {
        // if (node->children[i].move == MOVE_NONE)
        //     continue;
        auto v = children[i].n_visits;

        if (debug_best_visits)
            std::cerr << "Move " << children[i].move << " with " << v << " visits and mean value " << children[i].avg_action_value << std::endl;

        if (v > best_val)
        {
            best_val = v;
            best = i;
        }
    }

    if (debug_best_visits)
        std::cerr << "Returning with move " << node->children[best].move << std::endl;

    return &(node->children[best]);
}


//***************************** Playing moves ******************************/

Node* Agent::current_node()
{
    return nodes[ply];
}

bool Agent::is_terminal(Node* node)
{
    assert(node == current_node());
    return key_terminal(states[ply].key);
}

void Agent::init_children()
{
    //auto& children      = current_node()->children_list();
    assert(current_node()->n_children == 0);

    if (debug_init_children)
    {
        std::cerr << "Initializing children of node\n";
        Debug::display(std::cerr, state);
        std::cerr << std::endl;
    }

    // Make a local copy because the state's valid_actions
    // container will change during the random simulations.
    auto valid_actions = state.valid_actions();

    if (debug_init_children)
    {
        std::cerr << "With moves ";
        for (const auto& a : valid_actions) { std::cerr << a << ' '; }
        std::cerr << std::endl;
    }

    ++explored_nodes_cnt;

    for (auto move : valid_actions)
    {
        assert(move != MOVE_NONE);

        // std::cerr << "Starting random simulation at ply " << ply << " and state \n";
        // Debug::display(std::cerr, state);
        // std::cerr << std::endl;

        Reward prior = random_simulation(move);
        ++rollout_cnt;

        ActionNode new_action;
        new_action.move = move;
        new_action.n_visits = 0;
        new_action.prior_value = prior;
        new_action.action_value = 0;
        new_action.avg_action_value = 0;

        if (current_node()->n_children < MAX_CHILDREN)
        {
            current_node()->children_list()[current_node()->n_children] = new_action;
            ++(current_node()->n_children);
        }
    }

    if (debug_init_children)
        std::cerr << "initialized all children" << std::endl;

    ++current_node()->n_visits;

    std::sort(current_node()->children_list().begin(), current_node()->children_list().begin() + current_node()->n_children, [](const auto& a, const auto& b){
            return a.prior_value > b.prior_value;
        });

    // A sentinel to easily iterate through children.
    if (current_node()->n_children < MAX_CHILDREN)
    {
        ActionNode new_action;
        new_action.move = MOVE_END;
        new_action.n_visits = 0;
        new_action.prior_value = 0;
        new_action.action_value = 0;
        new_action.avg_action_value = 0;

        current_node()->children_list()[current_node()->n_children] = new_action;
    }
}

void Agent::apply_move(Move move)
{
    ++ply;
    state.apply_move(move, states[ply]);
}

void Agent::undo_move()
{
    --ply;
    state.undo_move(actions[ply]->move);
}

void Agent::undo_move(Move move)
{
    --ply;
    state.undo_move(move);
}

//***************************** Evaluation of nodes **************************/

// TODO I need the search stack to record which moves I need to undo!!
//
// First idea would be to not save or lookup anything, but save the next-to-last state
// (really, state-before-known-win/lose) to start building a table of alphas and betas.
Reward Agent::random_simulation(Move move)
{
    if (debug_random_sim)
    {
        Debug::display(std::cerr, state);
        std::cerr << std::endl;
        std::cerr << "Random simulation, ply " << ply << ", next move is " << move << std::endl;
    }

    stackBuf[ply].ply = ply;
    stackBuf[ply].currentMove = move;
    stackBuf[ply].r = 0;

    apply_move(move);

    if (key_terminal(states[ply].key))
    {
        if (debug_random_sim)
        {
            Debug::display(std::cerr, state);
            std::cerr << std::endl;
            std::cerr << "Reached terminal state at ply " << ply << std::endl;
            std::cerr << "Evaluation : " << std::fixed << key_ev_terminal(states[ply])  << std::endl;
        }

        stackBuf[ply-1].r += key_ev_terminal(states[ply]);
    } else
    {
        int n = state.valid_actions().size();
        Move m;
        if (n == 0)
            m = MOVE_NONE;
        else
            m = state.valid_actions()[rand() % n];

        stackBuf[ply-1].r += 1 - random_simulation(m);
    }

    undo_move(stackBuf[ply-1].currentMove);

    if (debug_random_sim)
    {
        std::cerr << "After undoing move " << stackBuf[ply].currentMove << ", evaluation at ply " << ply << ": " << stackBuf[ply].r << std::endl;
        std::cerr << "State's ply : " << state.gamePly << std::endl;
    }

    return stackBuf[ply].r;
}

void Agent::print_node(std::ostream& _out, Node* node) const
{
    _out << "Node: v=" << node->n_visits << std::endl;
    for (int i=0; i<node->n_children; ++i)
    {
        auto c = node->children[i];
        _out << "    Move " << c.move << ": v=" << c.n_visits << ", val=" << c.avg_action_value << std::endl;
    }
}

void Agent::print_tree(std::ostream& _out, int depth) const
{
    print_node(_out, root);
}

// Reward Agent::evaluate_terminal()
// {
//     Key key = states[ply].statusKey;

//     Reward r = key >> 2 & 1 ? 0
//                             : key >> 1 & 1 ? (1 - (ply & 1))
//                             : ply & 1;
//     return r;
// }

} // namespace mcts
