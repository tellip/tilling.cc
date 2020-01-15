cmake_minimum_required(VERSION 3.9)
project(tilling)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lxcb -lpthread")

add_executable(tilling src/main.cc)