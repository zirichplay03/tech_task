cmake_minimum_required(VERSION 3.10)

project(tech_task)

set(CMAKE_CXX_STANDARD 11)

add_executable(tech_task main.cpp)

configure_file(inlet.in inlet.in COPYONLY)