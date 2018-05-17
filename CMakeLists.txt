cmake_minimum_required(VERSION 3.9)
project(matrix_wm_2_cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11")

add_executable(wm wm.cc)
#add_executable(wm-control swap/wm-control.cc)