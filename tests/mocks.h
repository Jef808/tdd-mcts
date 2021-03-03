#include "gmock/gmock.h"

namespace mcts {

class MockNodeAction {
public:
    MOCK_CONST_METHOD0(expanded, bool(void));
    MOCK_METHOD0(mark_expanded, void(void));
};

class MockState {
public:
    using action_t = int;
    MOCK_CONST_METHOD0(valid_actions, std::vector<action_t>());
};



} // mcts
