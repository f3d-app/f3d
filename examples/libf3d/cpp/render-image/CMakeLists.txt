cmake_minimum_required(VERSION 3.10)

project(render-image)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(render-image main.cxx)
target_link_libraries(render-image f3d::libf3d)
set_target_properties(render-image PROPERTIES CXX_STANDARD 17)

add_executable(check-image check.cxx)
target_link_libraries(check-image f3d::libf3d)
set_target_properties(check-image PROPERTIES CXX_STANDARD 17)
