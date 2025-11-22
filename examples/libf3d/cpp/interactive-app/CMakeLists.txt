cmake_minimum_required(VERSION 3.10)

project(interactive-app)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(interactive-app main.cxx)
target_link_libraries(interactive-app f3d::libf3d)
set_target_properties(interactive-app PROPERTIES CXX_STANDARD 17)
