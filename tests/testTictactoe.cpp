#include "gmock/gmock.h"
#include "tictactoe.h"


using namespace testing;
using namespace ttt;

class TictactoeTest : public Test {
public:
    State stateInitial;

    State::grid_t emptyGrid {
        std::nullopt, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt,
    };
};

TEST_F(TictactoeTest, DefaultCtorInitializesAnEmptyGrid)
{
    ASSERT_THAT(stateInitial.grid(), ContainerEq(emptyGrid));
}

int main(int argc, char *argv[]) {

    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
