cmake_minimum_required(VERSION 3.15)
project(tilling_cc)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lxcb -lpthread")

add_executable(tilling src/main.cc)