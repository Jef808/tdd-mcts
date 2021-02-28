#include "gmock/gmock.h"
#include "tictactoe.h"


using namespace testing;
using namespace ttt;

class TictactoeTest : public Test {


};

TEST_F(TictactoeTest, DefaultCtorInitializesAnEmptyBoard)
{
    ttt::State state;
    ASSERT_THAT(state.get_grid(), Each(Token::EMPTY));
}



int main(int argc, char *argv[]) {

    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
