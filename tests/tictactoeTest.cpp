#include "gmock/gmock.h"
#include "tictactoe.h"
#include <utility>


using namespace testing;
using namespace ttt;

class TictactoeTest : public Test {

    private:


    public:

        State default_state;

        std::array<Token, 9> grid1 {{Token::X, Token::EMPTY, Token::EMPTY, Token::EMPTY, Token::X, Token::O, Token::O, Token::EMPTY, Token::X}};

        //State state(grid1);

};

TEST_F(TictactoeTest, DefaultCtorInitializesAnEmptyBoard)
{
    ASSERT_THAT(default_state.get_grid(), Each(Token::EMPTY));
}

TEST_F(TictactoeTest, CanInitializeStateWithGrid)
{
    auto state = State(grid1);

    ASSERT_THAT(state.get_grid(), ContainerEq(grid1));
}



int main(int argc, char *argv[]) {

    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
