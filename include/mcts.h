#ifndef __MCTS_H_
#define __MCTS_H_

#include <unordered_map>
#include "tictactoe.h"

namespace mcts {

struct ActionNode;
struct Node;


typedef double Reward;

class Agent {

public:
    static const int MAX_PLY = 128;
    static const int MAX_CHILDREN = 128;

    Agent(State& state);

    Move MCTSBestMove();

    void create_root();
    Node* tree_policy();
    Reward rollout_policy(Node* node);
    void backpropagate(Node* node, Reward r);

    Node* current_node();
    bool is_root(Node* root);
    bool is_terminal(Node* node);
    void do_move(Move move);
    void undo_move();
    void init_children();

    Reward random_simulation(Move move);
    Reward evaluate_terminal();

private:
    State& state;
    Node*   root;

    int ply;
    int iteration_cnt;

    // To keep track of nodes during the search (indexed by ply)
    std::vector<Node*>     nodes;
};

struct ActionNode {
    Move move;
    int n_visits;
    Reward prior_value;
    Reward action_value;
    Reward avg_action_value;
};

struct Node {
    using cont_children = std::vector<ActionNode>;
public:
    Key                 key                              = 0;           // Zobrist Hash of the state
    Move                last_move                        = MOVE_NONE;
    int                 n_visits                         = 0;
    int                 n_children                       = 0;
    int                 n_expanded_children              = 0;
    cont_children       children;

    cont_children& children_list() { return children; }
    cont_children::iterator begin() { return children.begin(); }
    cont_children::iterator end() { return children.end(); }
};

inline bool operator==(const Node& a, const Node& b)
{
    return a.key == b.key;
}

typedef std::unordered_map<Key, Node> MCTSLookupTable;

extern MCTSLookupTable MCTS;

}

#endif // __MCTS_H_
