cmake_minimum_required(VERSION 3.10)

project(use-options-struct)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(use-options-struct main.cxx)
target_link_libraries(use-options-struct f3d::libf3d)
set_target_properties(use-options-struct PROPERTIES CXX_STANDARD 17)
