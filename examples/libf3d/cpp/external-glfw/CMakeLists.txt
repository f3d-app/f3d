cmake_minimum_required(VERSION 3.10)

project(external-glfw)

find_package(f3d REQUIRED COMPONENTS library)
find_package(glfw3 REQUIRED)

add_executable(external-glfw main.cxx)
target_link_libraries(external-glfw f3d::libf3d glfw)
set_target_properties(external-glfw PROPERTIES CXX_STANDARD 17)
