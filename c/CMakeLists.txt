cmake_minimum_required(VERSION 3.21)

project(f3d_c_api)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(f3d_c_api SHARED f3d_c_api.c)

target_link_libraries(f3d_c_api PRIVATE f3d::libf3d)