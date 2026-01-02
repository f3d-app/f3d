cmake_minimum_required(VERSION 3.10)

project(interactive-app-c)

find_package(f3d REQUIRED COMPONENTS c_api)

add_executable(interactive-app-c main.c)

target_link_libraries(interactive-app-c
  PRIVATE
  f3d::c_api
)
