
cmake_minimum_required(VERSION 3.10)
file(GLOB SOURCES "*.cpp")
add_library(utils ${SOURCES})
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
