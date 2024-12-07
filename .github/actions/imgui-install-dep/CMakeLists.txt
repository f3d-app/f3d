cmake_minimum_required(VERSION 3.16)
project(imgui CXX)

add_library(${PROJECT_NAME} "")
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        IMGUI_DISABLE_DEMO_WINDOWS
        IMGUI_DISABLE_DEBUG_TOOLS
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
        POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_target
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imstb_textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imstb_rectpack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imstb_truetype.h
    DESTINATION include
)

include(CMakePackageConfigHelpers)
configure_package_config_file(imgui-config.cmake.in imgui-config.cmake INSTALL_DESTINATION share/imgui)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/imgui-config.cmake DESTINATION share/imgui)

install(
    EXPORT ${PROJECT_NAME}_target
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}-targets.cmake
    DESTINATION share/${PROJECT_NAME}
)
