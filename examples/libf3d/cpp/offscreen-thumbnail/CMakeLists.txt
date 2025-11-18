cmake_minimum_required(VERSION 3.10)

project(offscreen-thumbnail)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(offscreen-thumbnail main.cxx)
target_link_libraries(offscreen-thumbnail f3d::libf3d)
set_target_properties(offscreen-thumbnail PROPERTIES CXX_STANDARD 17)
