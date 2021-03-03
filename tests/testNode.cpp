#include "mocks.h"
#include "node.h"
#include "tictactoe.h"
#include "gmock/gmock.h"
#include "gtest/gtest-matchers.h"
#include "gtest/gtest.h"

namespace mcts {

namespace {

    struct FakeState {

        using action_t = int;
        FakeState()
            : mock_state(nullptr)
        {
        }
        void init(MockState* mock)
        {
            mock_state = mock;
        }
        std::vector<action_t> valid_actions() const
        {
            return mock_state->valid_actions();
        }
        MockState* mock_state;
    };

    class TestEnvironment : public ::testing::Environment {
    };

    class NodeTest : public ::testing::Test {
    protected:
        NodeTest()
            : fake_state()
        {
        }
        using NodeT = mcts::Node<FakeState>;

        FakeState fake_state;
    };

    class NodeActionsTest : public NodeTest {
    protected:
        using Q = MockState::action_t;
        using R = NodeT::naction_t;
        void SetUp()
        {
            state_actions = { Q{0}, 1, 2, 3 };
            node_actions = { R{0, false}, {1, false}, {2, false}, {3, false} };
            node_actions_toggled = { R{0, false}, {1, false}, {2, true}, {3, false} };

        }

        std::vector<Q> state_actions;
        std::vector<R> node_actions;
        std::vector<R> node_actions_toggled;
};


    using namespace testing;

    //ACTION(stateActions) { return NodeActionsTest::state_actions; };



    TEST_F(NodeActionsTest, NodeSavesValidActionsOnInitialize)
    {
        MockState mock_state; fake_state.init(&mock_state);
        EXPECT_CALL(mock_state, valid_actions()).Times(1).WillOnce(Return(state_actions));
        NodeT node { fake_state };
        ASSERT_THAT(node.valid_actions(), ContainerEq(node_actions));
    }

    TEST_F(NodeActionsTest, ExpandingAnActionTurnsOnItsExpandedField)
    {
        MockState mock_state; fake_state.init(&mock_state);
        EXPECT_CALL(mock_state, valid_actions()).Times(1).WillOnce(Return(state_actions));
        NodeT node { fake_state };
        node.expand_action(node.valid_actions()[1]);
        ASSERT_THAT(node.valid_actions(), ContainerEq(node_actions_toggled));
    }

    TEST_F(NodeTest, UnexpandedActionsReturnsActionsUnexpanded)
    {
        MockState mock_state;

        EXPECT_CALL(mock_state, valid_actions());
        ON_CALL(mock_state, valid_actions()).WillByDefault([]() {
            return std::vector<int> { 1, 2, 3 };
        });

        fake_state.init(&mock_state);
        NodeT node { fake_state };
        node.expand_action(1);

        using T = NodeT::naction_t;
        ASSERT_THAT(node.unexp_actions(),
                    ElementsAreArray({ T{1, false}, {3, false}  }));
    }


} // namespace
} // namespace ttt

int main(int argc, char* argv[])
{
    testing::AddGlobalTestEnvironment(new mcts::TestEnvironment());
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
