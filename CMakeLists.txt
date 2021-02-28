project(MCTS-TDD)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
add_definitions(-std=c++0x)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall -g")

set(sources_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(headers_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(tests_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/tests)

set(tictactoe_sources
  ${sources_dir}/tictactoe.cpp
  ${headers_dir}/tictactoe.h)

set(test_sources
  ${tests_dir}/main.cpp
  ${tests_dir}/tictactoeTest.cpp
  ${headers_dir}/tictactoe.h)

add_executable(tictactoe ${tictactoe_sources})
target_include_directories(tictactoe PUBLIC ${sources_dir} ${headers_dir})

add_executable(test ${test_sources})
target_include_directories(test PUBLIC ${sources_dir} ${tests_dir} ${headers_dir})
target_link_libraries(test pthread)
target_link_libraries(test gmock)
target_link_libraries(test gtest)
