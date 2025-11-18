cmake_minimum_required(VERSION 3.10)

project(libf3d-examples)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(cpp)
