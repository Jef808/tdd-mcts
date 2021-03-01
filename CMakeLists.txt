project(TDD-MCTS)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
#add_definitions(-std=c++0x -stdlib=libc++)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
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
  ${sources_dir}/tictactoe.cpp
  ${headers_dir}/tictactoe.h)

add_library(tictactoe ${tictactoe_sources})
target_include_directories(tictactoe PUBLIC ${sources_dir} ${headers_dir})

set(testState_sources
  ${tests_dir}/testState.cpp
  )

add_executable(testState ${testState_sources})
target_link_libraries(testState tictactoe)
target_include_directories(testState PUBLIC ${sources_dir} ${tests_dir} ${headers_dir})
target_link_libraries(testState pthread)
target_link_libraries(testState gmock)
target_link_libraries(testState gtest)
