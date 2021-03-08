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
    Node* selection_policy();
    ActionNode& expansion_policy();
    Reward rollout_policy(Node* node);
    void backpropagate(Node* node, Reward r);

    Node* current_node();
    bool is_root(Node* root);
    void do_move(Move move);
    void undo_move();
    void generate_actions();
    void init_actionchild(Node* node, Move move, Reward rollout_score, int move_cnt);

private:
    State& state;
    Node*   root;

    int ply;
    int iteration_cnt;

    // To store the search data locally
    std::vector<Node*> node_buffer;
    std::vector<StateData> node_stack;

};

struct ActionNode {
    Move move;
    int n_visits;
    Reward first_rollout;
    Reward action_value;
    Reward avg_action_value;
};

struct Node {
    using cont_children = std::array<ActionNode, Agent::MAX_CHILDREN>;
public:
    Key                 key                              = 0;           // Zobrist Hash of the state
    Move                last_move                        = MOVE_NULL;
    int                 n_visits                         = 0;
    int                 n_children                       = 0;
    int                 n_expanded_children              = 0;
    //ActionNode          children[Agent::MAX_PLY];
    cont_children children;

    //std::vector<ActionNode>& children_list() { return children; }
    cont_children::iterator begin() { return children.begin(); }
    cont_children::iterator end() { return children.end(); }
};

typedef std::unordered_map<Key, Node> MCTSLookupTable;

extern MCTSLookupTable MCTS;

}

#endif // __MCTS_H_
