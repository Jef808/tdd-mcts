#include "mocks.h"
#include "node.h"
#include "tictactoe.h"
#include "gmock/gmock.h"
#include "gtest/gtest-matchers.h"
#include "gtest/gtest.h"

namespace mcts {

namespace {

    class TestEnvironment : public ::testing::Environment {
    };

    class NodeTest : public ::testing::Test {
    protected:
        NodeTest()
            : fake_state()
        {
        }
        using NodeStateT = mcts::NodeState<FakeState>;
        using Q = MockState::action_t;
        using R = NodeStateT::naction_t;
        void SetUp()
        {
            state_actions0123 = { Q{0}, 1, 2, 3 };
            state_actions013 = { Q{0}, 1, 3 };
            node_actions0123 = { R{0, false}, {1, false}, {2, false}, {3, false} };
            node_actions_2toggled = { R{0, false}, {1, false}, {2, true}, {3, false} };
            node_actions_013unexpanded = { R{0, false}, {1, false}, {3, false} };
        }
        std::vector<Q> state_actions0123;
        std::vector<Q> state_actions013;
        std::vector<R> node_actions0123;
        std::vector<R> node_actions_2toggled;
        std::vector<R> node_actions_013unexpanded;

        FakeState fake_state;
    };

    using namespace testing;

    TEST_F(NodeTest, NodeSavesValidActionsOnInitialize)
    {
        MockState mock_state; fake_state.init(&mock_state);
        EXPECT_CALL(mock_state, valid_actions()).Times(1).WillOnce(Return(state_actions0123));

        NodeStateT node { fake_state };
        ASSERT_THAT(node.valid_actions(), ContainerEq(node_actions0123));
    }

    TEST_F(NodeTest, UnexpandedActionsReturnsActionsUnexpanded)
    {
        MockState mock_state; fake_state.init(&mock_state);
        EXPECT_CALL(mock_state, valid_actions()).Times(2).WillOnce(Return(state_actions0123))
                                                         .WillOnce(Return(state_actions013));

        NodeStateT node { fake_state };
        node.expand_action(2);
        ASSERT_THAT(node.unexpanded_actions(), ContainerEq(node_actions_013unexpanded));
    }

    TEST_F(NodeTest, ExpandingAnActionTogglesOnItsExpandedField)
    {
        MockState mock_state; fake_state.init(&mock_state);
        EXPECT_CALL(mock_state, valid_actions()).Times(2).WillOnce(Return(state_actions0123))
                                                         .WillOnce(Return(state_actions013));

        NodeStateT node { fake_state };
        node.expand_action(2);
        ASSERT_THAT(node.valid_actions(), ContainerEq(node_actions_2toggled));
    }

    // TEST_F(NodeTest, ExpandingUsesActionToCreateNewState)
    // {
    //     MockNodeAction mock_action;
    //     MockState mock_state;
    //     fake_state.init(&mock_state);

    //     EXPECT_CALL(mock_state, )
    //     EXPECT_CALL(mock_action, act(mock_state));
    // }





} // namespace
} // namespace ttt

int main(int argc, char* argv[])
{
    testing::AddGlobalTestEnvironment(new mcts::TestEnvironment());
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
