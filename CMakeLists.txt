project(MCTS-TDD)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
add_definitions(-std=c++0x)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall -g")

set(tictactoe_sources
  src/tictactoe.cpp
  include/tictactoe.h)

add_executable(tictactoe ${tictactoe_sources})

set(test_sources
  tests/main.cpp
  tests/tictactoeTest.cpp)

add_executable(test ${test_sources})
target_include_directories(test PUBLIC ./tests)
target_link_libraries(test pthread)
target_link_libraries(test gmock)
target_link_libraries(test gtest)
