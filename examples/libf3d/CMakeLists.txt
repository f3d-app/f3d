cmake_minimum_required(VERSION 3.5)

project(libf3d-examples)

find_package(f3d REQUIRED)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(check-engine)
add_subdirectory(multi-geom)
add_subdirectory(render-image)
add_subdirectory(render-interact)
