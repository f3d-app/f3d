cmake_minimum_required(VERSION 3.19)

project(libf3d-examples)

find_package(f3d REQUIRED)

# Check Plugin SDK is available
if(NOT (DEFINED f3d_SDK_PLUGIN AND ${f3d_SDK_PLUGIN}))
  message(FATAL_ERROR "f3d_SDK_PLUGIN is not enabled, can't configure plugin")
endif()
message(STATUS "f3d_SDK_PLUGIN is enabled")

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(example-plugin)
