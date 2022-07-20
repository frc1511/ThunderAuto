cmake_minimum_required(VERSION 3.1)
project(stb_image)

add_library(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.c")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")