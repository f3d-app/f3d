cmake_minimum_required(VERSION 3.19)

project(libf3d-examples)

find_package(f3d REQUIRED COMPONENTS pluginsdk)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(example-plugin)
