cmake_minimum_required(VERSION 3.9)
project(wm_2_cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11 -lpthread")

FILE(GLOB Public src/*.hh)
FILE(GLOB Server src/server/*.hh src/server/*.cc)
add_executable(matwm ${Public} ${Server})
add_executable(matc src/client.cc ${Public})
