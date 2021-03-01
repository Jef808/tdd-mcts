#include "gmock/gmock.h"

template<typename StateT>
class MockAction {
    MOCK_METHOD(StateT&, Call, (StateT&), (const));
    MOCK_METHOD(StateT, Call, (const StateT&), (const));
};
/**
 * @T_Params NdxT  : the type used to index (and access) the data.
 *           ValT  : the type used to represent the data.
 *           CntT  : the type of the container holding the data.
 */
template<typename NdxT, typename ValT, typename CntT>
class MockState {
public:
    MOCK_METHOD(bool, is_terminal, (), (const));
    MOCK_METHOD(CntT, valid_actions, (), (const));
};

template<typename ActionT, typename ChldCntT, typename ValT>
class MockNode {
public:
    MOCK_METHOD(ChldCntT&, children, (), (const));
    MOCK_METHOD(ValT, value, (), (const));
    MOCK_METHOD(int, n_visits, (), (const));
    MOCK_METHOD(bool, is_terminal, (), (const));
    MOCK_METHOD(bool, can_expand, (), (const));
    MOCK_METHOD(MockNode&, add_child, (const ActionT&));
    MOCK_METHOD(void, apply_action, (const ActionT&));
};
