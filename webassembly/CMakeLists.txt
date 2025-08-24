cmake_minimum_required(VERSION 3.17) # 3.17 required for CMP0099

# Need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10551
if(NOT VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231007)
  message(FATAL_ERROR "Cannot build emscripten bindings with VTK version below 9.3.20231007")
endif()

set(F3D_WASM_DATA_FILE "${F3D_SOURCE_DIR}/testing/data/f3d.vtp"
  CACHE PATH "File to copy in the webassembly root virtual filesystem")

add_executable(f3djs F3DEmscriptenBindings.cxx)

set_target_properties(f3djs PROPERTIES
  OUTPUT_NAME f3d
)

target_link_libraries(f3djs PRIVATE libf3d)

target_link_options(f3djs PRIVATE
  "$<IF:$<CONFIG:Release>,-Oz,-O0>"
  "--bind"
  "--closure 1"
  "SHELL:--preload-file ${F3D_WASM_DATA_FILE}@/"
  "SHELL:-sEXPORT_NAME=f3d"
  "SHELL:-sALLOW_MEMORY_GROWTH=1"
  "SHELL:-sEMULATE_FUNCTION_POINTER_CASTS=0"
  "SHELL:-sMODULARIZE=1"
  "SHELL:-sUSE_PTHREADS=0"
  "SHELL:-sSTACK_SIZE=1048576"
  "SHELL:-sWASM=1"
  "SHELL:-sFORCE_FILESYSTEM"
  "SHELL:-sEXPORTED_RUNTIME_METHODS=FS"
  "SHELL:-sWASM_BIGINT"
)

# Option to copy app.html file to index.html in the binary folder
# In order to test the app:
# - go to the binary folder where f3d.js, f3d.wasm and f3d.data are located
# - run `python -m http.server 8000`
# - open a browser to http://localhost:8000
option(F3D_WASM_COPY_APP "Copy HTML app file to binary folder" ON)

if(F3D_WASM_COPY_APP)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/app.html"
    "${CMAKE_BINARY_DIR}/bin/index.html"
    COPYONLY)
  configure_file(
    "${F3D_SOURCE_DIR}/resources/logo.ico"
    "${CMAKE_BINARY_DIR}/bin/favicon.ico"
    COPYONLY)
  configure_file(
    "${F3D_SOURCE_DIR}/resources/logo256.png"
    "${CMAKE_BINARY_DIR}/bin/icon256.png"
    COPYONLY)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
    "${CMAKE_BINARY_DIR}/bin/manifest.json"
    COPYONLY)
endif()
