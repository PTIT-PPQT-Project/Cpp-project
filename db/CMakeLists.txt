
cmake_minimum_required(VERSION 3.10)
file(GLOB SOURCES "*.cpp")
add_library(db ${SOURCES})
target_include_directories(db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
