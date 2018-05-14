cmake_minimum_required(VERSION 3.9)
project(matrix_wm_2_cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lxcb")

add_executable(wm wm/main.cc)
add_executable(wm_client wm_client/main.cc)