cmake_minimum_required(VERSION 3.26)
project(Proyecto_1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Proyecto_1
        AVLFile.h
        main.cpp
        SequentialFile.h)
