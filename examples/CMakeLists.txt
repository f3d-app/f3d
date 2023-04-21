cmake_minimum_required(VERSION 3.19)

project(examples)

find_package(f3d REQUIRED)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(libf3d)
add_subdirectory(plugins)
