
cmake_minimum_required(VERSION 3.10)
file(GLOB SOURCES "*.cpp")
add_library(controller ${SOURCES})
target_include_directories(controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
