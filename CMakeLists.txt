cmake_minimum_required(VERSION 3.27)
project(GameAi)

set(CMAKE_CXX_STANDARD 17)

add_executable(GameAi main.cpp
#        MazeState.cpp
#        Greedy.cpp
#        BeamSearch.cpp
#        BeamSearchWithTime.cpp
        ChokudaiSearch.cpp)
