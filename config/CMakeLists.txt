
cmake_minimum_required(VERSION 3.10)
file(GLOB SOURCES "*.cpp")
add_library(config ${SOURCES})
target_include_directories(config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
