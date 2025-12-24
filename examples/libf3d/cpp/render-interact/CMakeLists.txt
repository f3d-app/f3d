cmake_minimum_required(VERSION 3.10)

project(render-interact)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(render-interact main.cxx)
target_link_libraries(render-interact f3d::libf3d)
set_target_properties(render-interact PROPERTIES CXX_STANDARD 17)
