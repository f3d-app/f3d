cmake_minimum_required(VERSION 3.10)

project(minimal_qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(f3d REQUIRED COMPONENTS library)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

set(SOURCES
  main.cxx
  F3DRenderer.cxx
  F3DRenderer.h
  F3DView.cxx
  F3DView.h
)

add_executable(minimal_qml ${SOURCES})
target_link_libraries(minimal_qml
  PRIVATE
  f3d::libf3d
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
)
set_target_properties(minimal_qml PROPERTIES CXX_STANDARD 17)

# Copy QML file next to the executable
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Main.qml"
  "${CMAKE_CURRENT_BINARY_DIR}/Main.qml"
  COPYONLY
)
