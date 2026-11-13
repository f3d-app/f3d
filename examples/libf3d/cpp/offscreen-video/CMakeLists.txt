cmake_minimum_required(VERSION 3.10)

project(offscreen-video)

find_package(f3d REQUIRED COMPONENTS library)

if (NOT f3d_MODULE_FFMPEG)
  message(FATAL_ERROR "FFmpeg module is required for this example")
endif()

add_executable(offscreen-video main.cxx)
target_link_libraries(offscreen-video f3d::libf3d)
set_target_properties(offscreen-video PROPERTIES CXX_STANDARD 17)
