cmake_minimum_required (VERSION 2.6)

project(mcpelauncherpatchesrpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_library(log-lib log)


add_library(mcpelauncherpatchesrpc SHARED src/main.cpp)
target_link_libraries(mcpelauncherpatchesrpc PUBLIC ${log-lib})
