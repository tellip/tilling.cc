cmake_minimum_required(VERSION 3.9)
project(wm_2_cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11 -lpthread")

FILE(GLOB Public *.hh)
FILE(GLOB Server server/*.hh server/*.cc)
add_executable(server ${Public} ${Server})
add_executable(client client.cc ${Public})