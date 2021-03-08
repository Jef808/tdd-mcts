#include <optional>
#include <typeinfo>
#include <utility>
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "tictactoe.h"

namespace mcts {
namespace {

    class TestEnvironment : public ::testing::Environment {

    public:
        // Usage of CreateGrid :
        //          testGridXWinner = { Token::X,     std::nullopt, Token::O,
        //                              std::nullopt, Token::X,     Token::O,
        //                              std::nullopt, std::nullopt, Token::X } };
        // Is expressed as
        //          testGridXWinner = CreateGrid({0, 4, 8}, {2, 5});
        //
        // i.e. the first set of indices are for the X tokens, the second for O tokens.

        using T = std::vector<int>;
        static State::grid_t CreateGrid(T Xs, T Os)
        {
            State::grid_t ret { };
            for (auto ndx : Xs) {
                ret[ndx] = X;
            }
            for (auto ndx : Os) {
                ret[ndx] = O;
            }
            return ret;
        }

        static std::vector<Move> ComplMove(const std::vector<Move>& actions)
        {
            static std::vector<Move> ret;
            int base_ndx = ((int)actions.back()) > 9 ? 10 : 1;

            for (auto i = base_ndx; i < base_ndx + 9; ++i)
            {
                if (std::find(begin(actions), end(actions), Move(i)) == end(actions)) {
                    ret.emplace_back(Move(i));
                }
            }
            return ret;
        }

        // void SetUp() { }

        static const inline State::grid_t testGridEmpty {{TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY}};
        static const inline State::grid_t testGridOneX {{X, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY}};
        static const inline State::grid_t testGridOneXO {{X, TOK_EMPTY, TOK_EMPTY, O, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY}};
    };

    class StateTest : public ::testing::Test {
    protected:
        StateTest()
            : initialState()
        {
        }

        State initialState;
     };

    using namespace ::testing;

    TEST_F(StateTest, CreateGridTestMethod)
    {
        State::grid_t expected {{ X, TOK_EMPTY, O, TOK_EMPTY, X, O, TOK_EMPTY, TOK_EMPTY, X }};
        auto grid = TestEnvironment::CreateGrid({0, 4, 8}, {2, 5});

        ASSERT_THAT(grid, ContainerEq(expected));
    }

    TEST_F(StateTest, ComplMoveTestMethod)
    {
        std::vector<Move> expected = { Move(3), Move(4), Move(6), Move(7), Move(8) };
        std::vector<Move> ndxs { Move(1), Move(2), Move(5), Move(9) };
        auto moves = TestEnvironment::ComplMove(ndxs);

        ASSERT_THAT(moves, ContainerEq(expected));
    }

    TEST_F(StateTest, StateDefaultCtorInitializesAnEmptyGrid)
    {
        ASSERT_THAT(initialState.grid(), ContainerEq(TestEnvironment::testGridEmpty));
    }

    TEST_F(StateTest, StateCanBeInitializedByMovingGrid)
    {
        auto grid = TestEnvironment::testGridOneX;
        auto testStateX = State(std::move(grid));
        ASSERT_THAT(testStateX.grid(), ContainerEq(TestEnvironment::testGridOneX));
    }

    TEST_F(StateTest, ApplyMovePlacesTokenAtIndex)
    {
        initialState.apply_move(Move(1));
        ASSERT_THAT(initialState.grid(), ContainerEq(TestEnvironment::testGridOneX));
    }

    TEST_F(StateTest, ApplyMoveIsComposable)
    {
        ASSERT_THAT(initialState.apply_move(Move(1)).apply_move(Move(13)).grid(), ContainerEq(TestEnvironment::testGridOneXO));
    }

    TEST_F(StateTest, IsTerminalTrueOnFullGrid)
    {
        State stateFullX (TestEnvironment::CreateGrid({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }, {}));

        ASSERT_THAT(stateFullX.is_terminal(), IsTrue());
    }

    TEST_F(StateTest, IsTerminalFalseWhenStillCanPlay)
    {
        State state { };
        std::vector<Move> actions { Move(11), Move(5), Move(10), Move(8) };

        auto it = begin(actions);
        auto apply_action = [&it, &state] () {
            state = state.apply_move(*it);
            ++it;
        };

        bool sentinel = false;

        while (it != end(actions))
        {
            sentinel = true;

            EXPECT_THAT(state.is_terminal(), IsFalse());
            apply_action();
        }

        if (!sentinel)
        {
            FAIL() << "Test IsTerminalFalseWhenStillCanPlay escaped the assertions!" << std::endl;
        }
    }

    TEST_F(StateTest, IsTerminalTrueOnWinner)
    {
        State winnerX ( TestEnvironment::CreateGrid({ 0, 4, 8 }, { 2, 5 }) );
        State winnerO ( TestEnvironment::CreateGrid({ 0, 4, 7 }, { 2, 5, 8 }) );

        EXPECT_THAT(winnerX.is_terminal(), IsTrue());
        EXPECT_THAT(winnerO.is_terminal(), IsTrue());
    }

    TEST_F(StateTest, EmptyCellsFullOnInitialState)
    {
        std::list<Cell> expected;
        for (int i=0; i<9; ++i)
        {
            expected.push_back(Cell(i));
        }

        ASSERT_THAT(initialState.empty_cells(), ContainerEq(expected));
    }

    TEST_F(StateTest, EmptyCellsCorrectlyInitializedWhenMovingGrid)
    {
        State state ( TestEnvironment::CreateGrid({ 0, 4, 7 }, { 2, 5 }) );
        std::list<Cell> expected;
        std::vector<int> expected_ndx {{ 1, 3, 6, 8 }};
        for (auto ndx : expected_ndx)
        {
            expected.push_back(Cell(ndx));
        }

        ASSERT_THAT(state.empty_cells(), ContainerEq(expected));
    }

    TEST_F(StateTest, ApplyingMoveRemovesCellFromEmptyCells)
    {
        initialState.apply_move(Move(5)).apply_move(Move(15));
        std::list<Cell> expected;
        std::vector<int> expected_ndx {{ 0, 1, 2, 3, 6, 7, 8}};
        for (auto ndx : expected_ndx)
        {
            expected.push_back(Cell(ndx));
        }

        ASSERT_THAT(initialState.empty_cells(), ContainerEq(expected));
    }

    TEST_F(StateTest, NextPlayerIsWorking)
    {
        auto state = initialState;
        EXPECT_THAT(state.next_player(), Eq(X));

        state.apply_move(Move(2));
        EXPECT_THAT(state.next_player(), Eq(O));

        state.apply_move(Move(12));
        EXPECT_THAT(state.next_player(), Eq(X));
    }

    TEST_F(StateTest, ValidActionForEachEmptyCell)
    {
        State state ( TestEnvironment::CreateGrid({ 0, 4, 7 }, { 2, 5 }) );

        auto empty_cells = state.empty_cells();
        std::vector<Move> expected {{ Move(11), Move(13), Move(16), Move(18) }};

        ASSERT_THAT(state.valid_actions(), ContainerEq(expected));
    }

    TEST_F(StateTest, ValidActionsReflectNextPlayerToken)
    {
        EXPECT_THAT(initialState.valid_actions(), Each(Lt(10)));
        initialState.apply_move(Move(1));
        EXPECT_THAT(initialState.valid_actions(), Each(Gt(9)));
    }


} // namespace
} // namespace mcts

int main(int argc, char* argv[])
{
    testing::AddGlobalTestEnvironment(new mcts::TestEnvironment());
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
