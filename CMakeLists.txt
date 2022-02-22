cmake_minimum_required(VERSION 3.22)

project(mini-pl-interpreter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(mini-pl src/main.cpp)
