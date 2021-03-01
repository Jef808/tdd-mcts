#include "tictactoe.h"
#include "gmock/gmock.h"
#include <utility>
#include <optional>

namespace ttt {
namespace {

    //ussing namespace ::testing;

    const State::grid_t EEmptyGrid = {
                std::nullopt, std::nullopt, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt
            };
    const State::grid_t EtestGridX = {
                Token::X, std::nullopt, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt
            };
    const State::grid_t EtestGridXO = {
                Token::X, std::nullopt, std::nullopt,
                std::nullopt, Token::O, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt
            };
    const State::grid_t EFullGrid = {
        Token::X, Token::X, Token::O,
        Token::O, Token::O, Token::X,
        Token::X, Token::O, Token::X
    };

    struct TestData : public ::testing::Environment {

    public:
        static State::grid_t EmptyGrid() {
            static const State::grid_t res = EEmptyGrid;
            return res;
        }
        static State::grid_t testGridX() {
            static const State::grid_t res = EtestGridX;
            return res;
        }
        static State::grid_t testGridXO() {
            static const State::grid_t res = EtestGridXO;
            return res;
        }
        static State::grid_t testFullGrid() {
            static const State::grid_t res = EFullGrid;
            return res;
        }

        virtual void SetUp() {
            EmptyGrid(); testGridX(); testGridXO(); testFullGrid();
        }
    };

    class StateTest : public ::testing::Test {
    protected:
        StateTest()
            : initialState()
            , stateHeap(new State())
            , action_X0(0, Token::X)
            , action_O4(4, Token::O)
            , emptyGrid(TestData::EmptyGrid())
            , testGridX(TestData::testGridX())
            , testGridXO(TestData::testGridXO())
            , testFullGrid(TestData::testFullGrid())
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
        State::grid_t emptyGrid;
        State::grid_t testGridX;
        State::grid_t testGridXO;
        State::grid_t testFullGrid;
    };

    template <typename T>
    class WithCounter : StateTest {
    public:
        static inline int objectsCreated = 0;
        static inline int objectsAlive = 0;
        WithCounter()
        {
            ++objectsCreated;
            ++objectsAlive;
        }
        WithCounter(const WithCounter&)
        {
            ++objectsCreated;
            ++objectsAlive;
        }

    protected:
        ~WithCounter()
        {
            --objectsAlive;
        }
    };

    using namespace ::testing;

    TEST_F(StateTest, StateDefaultCtorInitializesAnEmptyGrid)
    {
        //ASSERT_THAT(initialState.grid(), BeginEndDistanceIs(Eq(9)));
        ASSERT_THAT(initialState.grid(), ContainerEq(emptyGrid));
    }

    TEST_F(StateTest, StateCanBeInitializedByMovingGrid)
    {
        auto testStateX = State(std::move(testGridX));
        ASSERT_THAT(testStateX.grid(), ContainerEq(testGridX));
    }

    TEST_F(StateTest, ActionPlacesATokenAtIndex)
    {
        State state {action_X0(initialState)};
        ASSERT_THAT(state.grid(), ContainerEq(testGridX));
    }

    TEST_F(StateTest, IsTerminalTrueOnFullBoard)
    {
        State state {testFullGrid};
        ASSERT_THAT(State(std::move(testFullGrid)).is_terminal(), IsTrue());
    }

} // namespace
} // namespace ttt

int main(int argc, char* argv[])
{

    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
