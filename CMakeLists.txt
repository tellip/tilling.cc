cmake_minimum_required(VERSION 3.9)
project(wm_2_cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11 -lpthread")

FILE(GLOB Server src/*.hh src/server/*)
add_executable(matwm ${Server})
FILE(GLOB Client src/*.hh src/client.cc)
add_executable(matc ${Client})
