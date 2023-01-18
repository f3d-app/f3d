project(libf3d-examples)

cmake_minimum_required(VERSION 3.2)

find_package(f3d REQUIRED)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(check-engine)
add_subdirectory(render-image)
add_subdirectory(render-interact)
