#ifndef __MCTS_H_
#define __MCTS_H_

#include <chrono>
#include <unordered_map>
#include <iostream>
#include "tictactoe.h"
#include "search.h"
#include "type.h"

namespace mcts {

struct ActionNode;
struct Node;

template<class Entry, int Size>
struct HashTable {
  Entry* operator[](Key key) { return &table[(uint32_t)key & (Size - 1)]; }

private:
  std::vector<Entry> table = std::vector<Entry>(Size);
};

// struct SearchStack {
//     Move currentMove;
//     int ply;
//     Reward r;
// };

class Agent {

public:
    static const int MAX_PLY = 12;
    static const int MAX_CHILDREN = 10;
    static inline double exploration_cst = 0.7;
    static inline int MAX_TIME = 5000;    // In milliseconds.
    static inline int MAX_ITER = 1000;
    static inline bool use_time            = false;
    static inline bool propagate_minimax;

    Agent(State& state);

    Move MCTSBestMove();

    void create_root();
    bool computation_resources();
    Node* tree_policy();
    Reward rollout_policy(Node* node);
    void backpropagate(Node* node, Reward r);

    ActionNode* best_uct(Node* node);
    ActionNode* best_visits(Node* node);
    ActionNode* best_avg_val(Node* node);

    Node* current_node();
    bool is_root(Node* root);
    bool is_terminal(Node* node);
    void apply_move(Move move);
    void apply_move(Move move, StateData& sd);
    void undo_move();
    void undo_move(Move move);
    void init_children();

    Reward random_simulation(Move move);
    Reward evaluate_terminal();

    // Wether to backpropagate the minmax value of nodes or the rollout reward.
    static void set_backpropagate_minimax(bool);

    // Debugging
    void print_node(std::ostream&, Node*) const;
    void print_tree(std::ostream&, int depth) const;
    static void set_exp_c(double c);
    static void set_max_time(int t);
    static void set_max_iter(int i);
    static inline bool debug_counters      = true;
    static inline bool debug_main_methods  = false;
    static inline bool debug_tree          = false;
    static inline bool debug_best_visits   = false;
    static inline bool debug_init_children = false;
    static inline bool debug_random_sim    = false;

private:
    State& state;
    Node*   root;

    int ply;
    int iteration_cnt;

    int rollout_cnt;
    int descent_cnt;
    int explored_nodes_cnt;

    // To keep track of nodes during the search (indexed by ply)
    std::array<Node*, MAX_PLY>       nodes;       // The nodes.
    std::array<ActionNode*, MAX_PLY> actions;     // The actions.
    std::array<StateData, MAX_PLY>   states;      // Utility allowing state to do and undo actions.
    std::array<Search::Stack, MAX_PLY> stackBuf;  // Allows to perform independant without creading nodes.
};

struct ActionNode {
    Move                move;
    int                 n_visits;
    Reward              prior_value;
    Reward              action_value;
    double              avg_action_value;
    bool                decisive;                // If it is a known win etc...
};

struct Node {
    // Careful when iterating through children, an ActionNode object doesn't have
    // a 'zero' value (so may be initialized with random noise).
    // NOTE: after the init_children() method, the children will be ordered by
    // their à priori value `prior_value`.
    using cont_children = std::array<ActionNode, Agent::MAX_CHILDREN>;
public:
    Key                 key                              = 0;           // Zobrist Hash of the state
    int                 n_visits                         = 0;
    int                 n_children                       = 0;
    int                 n_expanded_children              = 0;
    Move                last_move                        = MOVE_NONE;
    bool                best_known;
    cont_children       children;

    cont_children& children_list() { return children; }
};

inline bool operator==(const Node& a, const Node& b)
{
    return a.key == b.key;
}

typedef std::unordered_map<Key, Node> MCTSLookupTable;

extern MCTSLookupTable MCTS;

}  // namespace mcts

#endif // __MCTS_H_
