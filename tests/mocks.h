#include "gmock/gmock.h"

namespace mcts {

class MockState {
public:
    using action_t = int;
    MOCK_CONST_METHOD0(valid_actions, std::vector<action_t>());
};

struct FakeState {
        using action_t = int;
        FakeState()
            : mock_state(nullptr)
        {
        }

        void init(MockState* mock) {
            mock_state = mock;
        }
        auto valid_actions() const {
            return mock_state->valid_actions();
        }
        MockState* mock_state;
    };

class MockNodeAction {
public:
    MOCK_CONST_METHOD0(expanded, bool(void));
    MOCK_METHOD0(mark_expanded, void(void));
    MOCK_CONST_METHOD1(act, FakeState(const FakeState&));
    MOCK_METHOD1(act, FakeState&(FakeState&));
};





} // mcts
