cmake_minimum_required(VERSION 3.22)

project(mini-pl-interpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

file(GLOB SOURCES "src/*.cpp")

add_executable(mini-pl ${SOURCES})
