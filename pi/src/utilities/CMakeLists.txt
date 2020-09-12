cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 14)
project(cnc)

get_filename_component(parent ../ ABSOLUTE)
include_directories(${parent})

message("parent directory: " ${parent})

set(libdocs	ConfigureUtility.cpp)

add_library(utilities ${libdocs})
