cmake_minimum_required(VERSION 3.13)
project(regular_cc)

set(CMAKE_CXX_STANDARD 17)

add_executable(test samples/test.cc)
add_executable(boson-convert samples/boson-convert/main.cc)
