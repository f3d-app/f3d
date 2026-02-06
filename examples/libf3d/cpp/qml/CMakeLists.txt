cmake_minimum_required(VERSION 3.10)

project(minimal_qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(f3d REQUIRED COMPONENTS library)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

set(SOURCES
  main.cxx
  F3DRenderer.cxx
  F3DRenderer.h
  F3DView.cxx
  F3DView.h
)

qt6_add_executable(${PROJECT_NAME} ${SOURCES} )
qt6_add_qml_module(${PROJECT_NAME}
    URI F3D
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        ${SOURCES}
    # do not add prefixes of directory structure to qrc path
    NO_RESOURCE_TARGET_PATH
)

target_link_libraries(${PROJECT_NAME} PRIVATE f3d::libf3d Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)

