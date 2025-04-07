
cmake_minimum_required(VERSION 3.10)
file(GLOB SOURCES "*.cpp")
add_library(view ${SOURCES})
target_include_directories(view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
