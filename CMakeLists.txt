cmake_minimum_required(VERSION 3.9)
project(matwm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lxcb -lpthread")

#FILE(GLOB Source src/*.h src/*.cc)
#add_executable(matwm src/main.matwm.cc)
#add_executable(matc src/main.matc.cc)

add_executable(matwm src/main.cc)