#ifndef __MCTS_H_
#define __MCTS_H_

#include <chrono>
#include <unordered_map>
#include "tictactoe.h"

namespace mcts {

struct ActionNode;
struct Node;

template<class Entry, int Size>
struct HashTable {
  Entry* operator[](Key key) { return &table[(uint32_t)key & (Size - 1)]; }

private:
  std::vector<Entry> table = std::vector<Entry>(Size);
};

typedef double Reward;

class Agent {

public:
    static const int MAX_PLY = 128;
    static const int MAX_CHILDREN = 128;
    const double exploration_cst = 0.7;
    const int MAX_TIME = 5000;    // In milliseconds.
    const int MAX_ITER = 10000;

    Agent(State& state);

    Move MCTSBestMove();

    void create_root();
    bool computation_resources();
    Node* tree_policy();
    Reward rollout_policy(Node* node);
    void backpropagate(Node* node, Reward r);

    ActionNode* best_uct(Node* node);
    ActionNode* best_visits(Node* node);

    Node* current_node();
    bool is_root(Node* root);
    bool is_terminal(Node* node);
    void apply_move(Move move);
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
    std::array<Node*, MAX_PLY>       nodes;
    std::array<ActionNode*, MAX_PLY> actions;
};

struct ActionNode {
    Move                move;
    int                 n_visits;
    Reward              prior_value;
    Reward              action_value;
    Reward              avg_action_value;
};

struct Node {
    using cont_children = std::array<ActionNode, Agent::MAX_CHILDREN>;
public:
    Key                 key                              = 0;           // Zobrist Hash of the state
    int                 n_visits                         = 0;
    int                 n_children                       = 0;
    int                 n_expanded_children              = 0;
    Move                last_move                        = MOVE_NONE;
    cont_children       children;

    cont_children& children_list() { return children; }
    cont_children::iterator begin() { return children.begin(); }
    cont_children::iterator end() { return children.begin() + n_children; }
};

inline bool operator==(const Node& a, const Node& b)
{
    return a.key == b.key;
}

typedef std::unordered_map<Key, Node> MCTSLookupTable;

extern MCTSLookupTable MCTS;

}

#endif // __MCTS_H_
