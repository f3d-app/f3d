cmake_minimum_required(VERSION 3.10)

project(use-options-string)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(use-options-string main.cxx)
target_link_libraries(use-options-string f3d::libf3d)
set_target_properties(use-options-string PROPERTIES CXX_STANDARD 17)
