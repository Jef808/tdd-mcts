#include "tictactoe.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <optional>
#include <typeinfo>
#include <utility>

namespace mcts {
namespace {

    std::ostream& operator<<(std::ostream& _out, Token token)
    {
        switch (token) {
        case TOK_EMPTY:
            _out << '*';
            break;
        case X:
            _out << 'X';
            break;
        case O:
            _out << 'O';
            break;
        }

        return _out;
    }

    std::ostream& operator<<(std::ostream& _out, const State& state)
    {
        auto grid = state.grid();
        for (int i = 0; i < 9; ++i) {
            _out << grid[i];
            if ((i + 1) % 3 == 0)
                _out << '\n';
        }
        return _out;
    }

    class TestEnvironment : public ::testing::Environment {
    public:
        static const inline State::grid_t testGridEmpty { { TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY } };
        static const inline State::grid_t testGridOneX { { X, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY, TOK_EMPTY } };
    };

    class StateTest : public ::testing::Test {
    protected:
        StateTest()
            : initialState()
        {
        }

        State initialState;

        using V = std::vector<int>;
        State CreateState(V Xs, V Os)
        {
            std::vector<Move> movesX;
            std::vector<Move> movesO;
            State state {};

            for (auto ndx : Xs) {
                movesX.push_back(Move(1 + ndx));
            }
            for (auto ndx : Os) {
                movesO.push_back(Move(10 + ndx));
            }

            for (int i = 0; i < movesO.size(); ++i) {
                state.apply_move(movesX[i], sd[2 * i]);
                state.apply_move(movesO[i], sd[2 * i + 1]);
            }
            if (movesX.size() > movesO.size())
                state.apply_move(movesX.back(), sd[2 * movesO.size()]);

            return state;
        }

        std::array<StateData, 40> sd;
    };

    using namespace ::testing;

    TEST_F(StateTest, CreateStateTestMethod)
    {
        State::grid_t expected { { X, TOK_EMPTY, O, TOK_EMPTY, X, O, TOK_EMPTY, TOK_EMPTY, X } };
        auto state = CreateState({ 0, 4, 8 }, { 2, 5 });

        ASSERT_THAT(state.grid(), ContainerEq(expected));
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
        initialState.apply_move(Move(1), sd[0]);
        ASSERT_THAT(initialState.grid(), ContainerEq(TestEnvironment::testGridOneX));
    }

    TEST_F(StateTest, IsTerminalTrueOnFullGrid)
    {
        State stateFullX(CreateState({ 0, 1, 2, 3, 4 }, { 5, 6, 7, 8 }));

        ASSERT_THAT(stateFullX.is_terminal(), IsTrue());
    }

    TEST_F(StateTest, IsTerminalFalseWhenStillCanPlay)
    {
        State state {};
        std::vector<Move> actions { Move(11), Move(5), Move(10), Move(8) };

        auto it = begin(actions);
        int i = 0;
        auto apply_action = [&]() mutable {
            state.apply_move(*it, sd[i]);
            ++it;
            ++i;
        };

        bool sentinel = false;

        while (it != end(actions)) {
            sentinel = true;

            EXPECT_THAT(state.is_terminal(), IsFalse());
            apply_action();
        }

        if (!sentinel) {
            FAIL() << "Test IsTerminalFalseWhenStillCanPlay escaped the assertions!" << std::endl;
        }
    }

    TEST_F(StateTest, IsTerminalTrueOnWinner)
    {
        State winnerX = CreateState({ 0, 4, 8 }, { 2, 5 });
        State winnerO = CreateState({ 0, 4, 7 }, { 2, 5, 8 });

        EXPECT_THAT(winnerX.is_terminal(), IsTrue());
        EXPECT_THAT(winnerO.is_terminal(), IsTrue());
    }

    TEST_F(StateTest, WinnerFunctionOkWithX)
    {
        // X*O
        // *XO
        // **x
        State winnerX = CreateState({ 0, 4, 8 }, { 2, 5 });
        ASSERT_THAT(winnerX.winner(), Eq(X));
    }

    TEST_F(StateTest, EmptyCellsFullOnInitialState)
    {
        std::list<Cell> expected;
        for (int i = 0; i < 9; ++i) {
            expected.push_back(Cell(i));
        }

        ASSERT_THAT(initialState.empty_cells(), ContainerEq(expected));
    }

    TEST_F(StateTest, EmptyCellsCorrectlyInitializedWithCreateState)
    {
        State state = CreateState({ 0, 4, 7 }, { 2, 5 });
        std::list<Cell> expected;
        std::vector<int> expected_ndx { { 1, 3, 6, 8 } };
        for (auto ndx : expected_ndx) {
            expected.push_back(Cell(ndx));
        }

        ASSERT_THAT(state.empty_cells(), ContainerEq(expected));
    }

    TEST_F(StateTest, ApplyingMoveRemovesCellFromEmptyCells)
    {
        initialState.apply_move(Move(5), sd[0]);
        initialState.apply_move(Move(15), sd[1]);
        std::list<Cell> expected;
        std::vector<int> expected_ndx { { 0, 1, 2, 3, 6, 7, 8 } };
        for (auto ndx : expected_ndx) {
            expected.push_back(Cell(ndx));
        }

        ASSERT_THAT(initialState.empty_cells(), ContainerEq(expected));
    }

    TEST_F(StateTest, NextPlayerIsWorking)
    {
        auto state = initialState;
        EXPECT_THAT(state.next_player(), Eq(X));

        state.apply_move(Move(2), sd[0]);
        EXPECT_THAT(state.next_player(), Eq(O));

        state.apply_move(Move(12), sd[1]);
        EXPECT_THAT(state.next_player(), Eq(X));
    }

    TEST_F(StateTest, ValidActionForEachEmptyCell)
    {
        State state(CreateState({ 0, 4, 7 }, { 2, 5 }));

        auto empty_cells = state.empty_cells();
        std::vector<Move> expected { { Move(11), Move(13), Move(16), Move(18) } };

        ASSERT_THAT(state.valid_actions(), ContainerEq(expected));
    }

    TEST_F(StateTest, ValidActionsReflectNextPlayerToken)
    {
        EXPECT_THAT(initialState.valid_actions(), Each(Lt(10)));
        initialState.apply_move(Move(1), sd[0]);
        EXPECT_THAT(initialState.valid_actions(), Each(Gt(9)));
    }

    TEST_F(StateTest, KeyNextPlayerWorks)
    {
        State state {};
        Key key = state.key();

        EXPECT_THAT(key_next_player(key), Eq(X));

        state.apply_move(Move(1), sd[0]);
        key = state.key();

        EXPECT_THAT(key_next_player(key), Eq(O));

        state.apply_move(Move(18), sd[1]);
        key = state.key();

        EXPECT_THAT(key_next_player(key), Eq(X));
    }

    TEST_F(StateTest, KeyTerminalWorks)
    {
        State state {};
        Key key = state.key();
        ASSERT_THAT(key_terminal(key), Eq(0));

        int sentinel = 0;
        // XOX
        // OXO
        // XO*
        std::vector<int> ndxs = { { 1, 11, 3, 13, 5, 15 } };
        for (int i = 0; i < 6; ++i) {
            state.apply_move(Move(ndxs[i]), sd[i]);
            key = state.key();

            EXPECT_THAT(key_terminal(key), Eq(0));
            ++sentinel;
        }

        if (sentinel < 6)
            FAIL() << "Test KeyTerminalWorks escaped the for loop!" << std::endl;

        state.apply_move(Move(7), sd[9]);
        key = state.key();

        EXPECT_THAT(key_terminal(key), 1);
    }

    TEST_F(StateTest, KeyWinnerWorksWithXAndO)
    {
        // XOX
        // OXO
        // XO*
        State state1 = CreateState({0, 2, 4, 6}, {1, 3, 5});
        // XX*
        // OOO
        // **X
        State state2 = CreateState({0, 1, 8}, {3, 4, 5});

        EXPECT_THAT(key_winner(state1.key()), Eq(X));
        EXPECT_THAT(key_winner(state2.key()), Eq(O));
    }

    TEST_F(StateTest, KeyWinnerWorksWithDraws)
    {
        // XXO
        // OOX
        // XOX
        State state1 = CreateState({ 0, 1, 5, 6, 8 }, { 2, 3, 4, 7 });
        // XXO
        // OXX
        // XOO
        State state2 = CreateState({ 0, 1, 4, 5, 6}, {2, 3, 7, 8});

        EXPECT_THAT(key_terminal(state1.key()), 1);
        //EXPECT_THAT(key_terminal(state2.key()), TOK_EMPTY);

        EXPECT_THAT(state1.winner(), TOK_EMPTY);
        EXPECT_THAT(state2.winner(), TOK_EMPTY);

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
