#include "mocks.h"
#include "tictactoe.h"
#include "gmock/gmock.h"
#include "gtest/gtest-matchers.h"
#include "gtest/gtest.h"
#include <optional>
#include <typeinfo>
#include <utility>

namespace ttt {
namespace {

    //ussing namespace ::testing;

    class TestEnvironment : public ::testing::Environment {

    public:
        static inline State::grid_t const testGridEmpty { std::nullopt, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt, std::nullopt };

        static inline State::grid_t const testGridOneX = { Token::X, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt, std::nullopt };

        static inline State::grid_t const testGridOneXO = { Token::X, std::nullopt, std::nullopt,
            std::nullopt, Token::O, std::nullopt,
            std::nullopt, std::nullopt, std::nullopt };

        // Usage of GridBuilder :
        //          testGridXWinner = { Token::X,     std::nullopt, Token::O,
        //                              std::nullopt, Token::X,     Token::O,
        //                              std::nullopt, std::nullopt, Token::X } };
        // Is expressed as
        //          testGridXWinner = GridBuilder({0, 4, 8}, {2, 5});
        //
        // i.e. the first set of indices are for the X tokens, the second for O tokens.
        static State::grid_t GridBuilder(std::vector<int> XsNdx, std::vector<int> OsNdx)
        {
            State::grid_t ret {};
            for (auto ndx : XsNdx) {
                ret[ndx] = Token::X;
            }
            for (auto ndx : OsNdx) {
                ret[ndx] = Token::O;
            }
            return ret;
        }

        static std::vector<int> projNdx(const std::vector<Action>& actions)
        {
            std::vector<int> ret {};

            for (const auto& action : actions) {
                ret.push_back(action.ndx());
            }
            return ret;
        }

        static auto projVal(const std::vector<Action>& actions)
        {
            std::vector<Action::value_t> ret;
            for (auto a : actions) {
                ret.push_back(a.val().value());
            }
            return ret;
        }

        static std::vector<int> complNdx(const std::vector<int>& actions_ndxs)
        {
            std::vector<int> ret {};
            for (auto i = 0; i < 9; ++i) {
                if (std::find(begin(actions_ndxs), end(actions_ndxs), i) == end(actions_ndxs)) {
                    ret.push_back(i);
                }
            }
            return ret;
        }

        static inline bool testProjNdx()
        {
            std::vector<Action> actions { { 0, Token::X }, { 2, Token::X }, { 5, Token::O } };

            static bool res = projNdx(actions) == std::vector<int>({ 0, 2, 5 });
            return res;
        }

        static inline bool testProjVal()
        {
            std::vector<Action> actions { { 0, Token::X }, { 2, Token::X }, { 5, Token::O } };

            const static bool res = projVal(actions) == std::vector<State::value_t>({ Token::X, Token::X, Token::O });
            return res;
        }

        static inline bool testComplNdx()
        {
            std::vector<int> ndxs { { 0, 2, 5, 6, 7 } };

            const static bool res = complNdx(ndxs) == std::vector<int>({ 1, 3, 4, 8 });
            return res;
        }

        void SetUp()
        {
            if (!testProjNdx()) {
                FAIL() << "projNdx Environment method failed test.";
            }
            if (!testProjVal()) {
                FAIL() << "projVal Environment method failed test.";
            }
            if (!testComplNdx()) {
                FAIL() << "complNdx Environment method failed test.";
            }
        }
    };

    class StateTest : public ::testing::Test {
    protected:
        StateTest()
            : initialState()
            , stateHeap(new State())
            , action_X0(0, Token::X)
            , action_O4(4, Token::O)
        {
        }
        ~StateTest() override
        {
            delete stateHeap;
        }
        // void TearDown() override { }
        State initialState;
        State* stateHeap;
        ttt::Action action_X0;
        ttt::Action action_O4;
    };

    using namespace ::testing;

    TEST_F(StateTest, StateDefaultCtorInitializesAnEmptyGrid)
    {
        //ASSERT_THAT(initialState.grid(), BeginEndDistanceIs(Eq(9)));
        ASSERT_THAT(initialState.grid(), ContainerEq(TestEnvironment::testGridEmpty));
    }

    TEST_F(StateTest, StateCanBeInitializedByMovingGrid)
    {
        auto testStateX = State(std::move(TestEnvironment::testGridOneX));
        ASSERT_THAT(testStateX.grid(), ContainerEq(TestEnvironment::testGridOneX));
    }

    TEST_F(StateTest, ActionPlacesTokenAtIndex)
    {
        State state { action_X0(initialState) };
        ASSERT_THAT(state.grid(), ContainerEq(TestEnvironment::testGridOneX));
    }

    TEST_F(StateTest, ActionMutableMutatesState)
    {
        State before {};
        auto after = Action(4, Token::O)(before);

        ASSERT_THAT(after.grid(), ContainerEq(before.grid()));
    }

    TEST_F(StateTest, ActionImmutableCreatesCopy)
    {
        const State before {};
        auto after = Action(4, Token::O)(before);

        ASSERT_THAT(after.grid(), Not(ContainerEq(before.grid())));
    }

    TEST_F(StateTest, ActionInPlaceIsComposable)
    {
        ASSERT_THAT(action_O4(action_X0(initialState)).grid(), ContainerEq(TestEnvironment::testGridOneXO));
    }

    TEST_F(StateTest, IsTerminalTrueOnFullGrid)
    {
        State stateFullX { TestEnvironment::GridBuilder({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }, {}) };
        ASSERT_THAT(stateFullX.is_terminal(), IsTrue());
    }

    TEST_F(StateTest, IsTerminalFalseWhenStillCanPlay)
    {

        State state {};
        std::array<ttt::Action, 5> actions { { ttt::Action(0, Token::X),
            ttt::Action(2, Token::O),
            ttt::Action(4, Token::X),
            ttt::Action(1, Token::O),
            ttt::Action(7, Token::X) } };

        std::array<State, 6> states {};
        states[0] = state;
        for (int i = 1; i < 6; ++i) {
            states[i] = actions[i - 1](states[i - i]);
        }

        std::array<bool, 6> results { true };

        for (int i = 0; i < 6; ++i) {
            results[i] = states[i].is_terminal();
        }

        ASSERT_THAT(results, Each(IsFalse()));
    }

    TEST_F(StateTest, IsTerminalTrueOnWinner)
    {
        State winnerX { TestEnvironment::GridBuilder({ 0, 4, 8 }, { 2, 5 }) };
        State winnerO { TestEnvironment::GridBuilder({ 0, 4, 7 }, { 2, 5, 8 }) };

        EXPECT_THAT(winnerX.is_terminal(), IsTrue());
        EXPECT_THAT(winnerO.is_terminal(), IsTrue());
    }

    TEST_F(StateTest, ValidActionNdxIfAndOnlyIfCellUnoccupied)
    {
        State state { TestEnvironment::GridBuilder({ 0, 4, 7 }, { 2, 5 }) };

        auto validActionsNdxs = TestEnvironment::projNdx(state.valid_actions());

        auto invalidActionsNdxs = TestEnvironment::complNdx(validActionsNdxs);

        EXPECT_THAT(validActionsNdxs, IsSupersetOf({ 1, 3, 6, 8 }));
        EXPECT_THAT(invalidActionsNdxs, IsSupersetOf({ 0, 4, 7, 2, 5 }));
    }

    TEST_F(StateTest, ValidActionTokenIsPlayerToPlay)
    {
        State state { TestEnvironment::GridBuilder({ 0, 4, 7 }, { 2, 5 }) };
        State stateSwap25 { TestEnvironment::GridBuilder({ 2, 4, 7 }, { 0, 5 }) };

        auto validActionsTokens = TestEnvironment::projVal(state.valid_actions());
        auto validActionsTokensSwap = TestEnvironment::projVal(stateSwap25.valid_actions());

        ttt::Action move { 1, Token::O };
        auto validActionsTokensAfterMove = TestEnvironment::projVal(move(state).valid_actions());

        EXPECT_THAT(validActionsTokens, Each(Token::O));
        EXPECT_THAT(validActionsTokensSwap, Each(Token::O));
        EXPECT_THAT(validActionsTokensAfterMove, Each(Token::X));
    }

} // namespace
} // namespace ttt

int main(int argc, char* argv[])
{
    testing::AddGlobalTestEnvironment(new ttt::TestEnvironment());
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
