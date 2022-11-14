cmake_minimum_required(VERSION 3.23)
project(myCDBv3 C)

set(CMAKE_C_STANDARD 99)

add_executable(myCDBv3 main.c common.h nodes.h util.h read.h meta.h split.h)
