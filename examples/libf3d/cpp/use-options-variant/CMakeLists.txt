cmake_minimum_required(VERSION 3.10)

project(use-options-variant)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(use-options-variant main.cxx)
target_link_libraries(use-options-variant f3d::libf3d)
set_target_properties(use-options-variant PROPERTIES CXX_STANDARD 17)
