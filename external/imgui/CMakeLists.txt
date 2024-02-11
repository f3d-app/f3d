# Dear ImGui
add_library(imgui INTERFACE)
add_library(imgui::imgui ALIAS imgui)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IMGUI_SOURCES
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp

    imgui_internal.h
    imgui.h

    # integration with opengl
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3_loader.h
)
list(TRANSFORM IMGUI_SOURCES PREPEND ${IMGUI_DIR}/)

target_sources(imgui INTERFACE ${IMGUI_SOURCES})
target_include_directories(imgui INTERFACE ${IMGUI_DIR})
target_include_directories(imgui INTERFACE ${IMGUI_DIR}/backends)
