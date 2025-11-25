cmake_minimum_required(VERSION 3.10)

project(minimal_qt6)

find_package(f3d REQUIRED COMPONENTS library)
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

# This example only works if libf3d was built against a VTK >= 9.3.20240914

add_executable(minimal_qt6 main.cxx)
target_link_libraries(minimal_qt6 f3d::libf3d Qt6::Widgets Qt6::OpenGLWidgets)
set_target_properties(minimal_qt6 PROPERTIES CXX_STANDARD 17)
