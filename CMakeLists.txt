cmake_minimum_required(VERSION 3.30)
project(test)

set(CMAKE_CXX_STANDARD 23)

add_executable(process-handling.cpp test.cpp)

add_executable(test test.cpp)