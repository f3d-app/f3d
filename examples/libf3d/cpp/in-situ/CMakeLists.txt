cmake_minimum_required(VERSION 3.10)

project(in-situ)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(in-situ main.cxx ClothSolver.cxx)
target_link_libraries(in-situ f3d::libf3d)
set_target_properties(in-situ PROPERTIES CXX_STANDARD 17)
