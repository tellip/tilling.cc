cmake_minimum_required(VERSION 3.9)
project(matrix_wm_2_cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11 -lpthread")

FILE(GLOB Headers *.hh)
add_executable(server main.server.cc ${Headers})
add_executable(client main.client.cc ${Headers})