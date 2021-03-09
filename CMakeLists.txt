project(TDD-MCTS)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

#add_definitions(-std=c++0x -stdlib=libc++)

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall -g")
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(sources_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(headers_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(tests_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/tests)

set(tictactoe_sources
  ${headers_dir}/type.h
  ${sources_dir}/tictactoe.cpp
  ${headers_dir}/tictactoe.h)

add_library(tictactoe ${tictactoe_sources})
target_include_directories(tictactoe PUBLIC ${sources_dir} ${headers_dir})

set(mcts_sources
  ${headers_dir}/type.h
  ${sources_dir}/mcts.cpp
  ${headers_dir}/mcts.h)

add_library(mcts ${mcts_sources})
target_link_libraries(mcts tictactoe)
target_include_directories(mcts PUBLIC ${sources_dir} ${headers_dir})

set(sources
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${headers_dir}/tictactoe.h
  ${sources_dir}/tictactoe.cpp
  ${headers_dir}/mcts.h
  ${sources_dir}/mcts.cpp
  ${headers_dir}/type.h)


add_executable(main ${sources})
target_link_libraries(main mcts tictactoe)
target_include_directories(main PUBLIC ${sources_dir} ${headers_dir})


 set(testState_sources
   ${tests_dir}/testState.cpp
   #${headers_dir}/tictactoe.h
   #${sources_dir}/tictactoe.cpp
   )

add_executable(testState ${testState_sources})
target_link_libraries(testState tictactoe)
#target_include_directories(testState PUBLIC ${tests_dir}) #${sources_dir} ${headers_dir})
target_link_libraries(testState pthread)
target_link_libraries(testState gmock)
target_link_libraries(testState gtest)
target_include_directories(testState PUBLIC $ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
target_link_directories(testState PUBLIC $ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
# set(testNode_sources
#   ${tests_dir}/testNode.cpp
#   ${tests_dir}/mocks.h
#   ${headers_dir}/node.h)

# add_executable(testNode ${testNode_sources})
# target_include_directories(testNode PUBLIC ${sources_dir} ${tests_dir} ${headers_dir})
# target_link_libraries(testNode pthread)
# target_link_libraries(testNode gmock)
# target_link_libraries(testNode gtest)

# add_custom_target(test_all testState testNode)
