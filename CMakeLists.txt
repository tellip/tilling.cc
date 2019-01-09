cmake_minimum_required(VERSION 3.9)
project(matwm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lxcb -lpthread")

FILE(GLOB Source src/*.h src/*.cc)
add_executable(matwm ${Client} src/main/matwm.cc)
add_executable(matc ${Client} src/main/matc.cc)
