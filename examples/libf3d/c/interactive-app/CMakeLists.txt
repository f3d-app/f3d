cmake_minimum_required(VERSION 3.10)

project(interactive-app-c)

find_package(f3d REQUIRED COMPONENTS library)

find_library(F3D_C_API_LIBRARY
  NAMES f3d_c_api
  REQUIRED
)

add_executable(interactive-app-c main.c)

target_link_libraries(interactive-app-c
  PRIVATE
  f3d::libf3d
  ${F3D_C_API_LIBRARY}
)
