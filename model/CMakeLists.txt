
cmake_minimum_required(VERSION 3.10)
file(GLOB SOURCES "*.cpp")
add_library(model ${SOURCES})
target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
