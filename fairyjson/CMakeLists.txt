cmake_minimum_required(VERSION 3.19)
project(fairyjson)

set(CMAKE_CXX_STANDARD 11)

add_executable(fairyjson fairy_json.h fairy_json.cpp test.cpp)