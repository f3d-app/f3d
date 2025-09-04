cmake_minimum_required(VERSION 3.17) # 3.17 required for CMP0099

# Need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10551
if(NOT VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231007)
  message(FATAL_ERROR "Cannot build emscripten bindings with VTK version below 9.3.20231007")
endif()

add_executable(f3djs F3DEmscriptenBindings.cxx)

set_target_properties(f3djs PROPERTIES
  OUTPUT_NAME f3d
)

target_link_libraries(f3djs PRIVATE libf3d)

target_link_options(f3djs PRIVATE
  "$<IF:$<CONFIG:Release>,-Oz,-O0>"
  "--bind"
  "--closure 1"
  "SHELL:-sEXPORT_NAME=f3d"
  "SHELL:-sALLOW_MEMORY_GROWTH=1"
  "SHELL:-sEMULATE_FUNCTION_POINTER_CASTS=0"
  "SHELL:-sMODULARIZE=1"
  "SHELL:-sEXPORT_ES6=1"
  "SHELL:-sUSE_PTHREADS=0"
  "SHELL:-sSTACK_SIZE=1048576"
  "SHELL:-sWASM=1"
  "SHELL:-sFORCE_FILESYSTEM"
  "SHELL:-sEXPORTED_RUNTIME_METHODS=FS"
  "SHELL:-sWASM_BIGINT"
)

enable_testing()
add_subdirectory(testing)
