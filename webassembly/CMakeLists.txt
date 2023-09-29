cmake_minimum_required(VERSION 3.12)

add_executable(f3djs F3DEmscriptenBindings.cxx)

set_target_properties(f3djs PROPERTIES
  OUTPUT_NAME f3d
)

target_link_libraries(f3djs PRIVATE libf3d)

target_link_options(f3djs PRIVATE
  "--bind"
  "-O1"
  "SHELL:--preload-file ${F3D_SOURCE_DIR}/testing/data@/"
  "SHELL:-sEXPORT_NAME=f3d"
  "SHELL:-sALLOW_MEMORY_GROWTH=1"
  "SHELL:-sDEMANGLE_SUPPORT=1"
  "SHELL:-sEMULATE_FUNCTION_POINTER_CASTS=0"
  "SHELL:-sERROR_ON_UNDEFINED_SYMBOLS=0"
  "SHELL:-sMODULARIZE=1"
  "SHELL:-sUSE_PTHREADS=0"
  "SHELL:-sWASM=1"
  "SHELL:-sASSERTIONS=1"
  "SHELL:-sFORCE_FILESYSTEM"
  "SHELL:-sFULL_ES3=1"
  "SHELL:-sUSE_SDL=2"
  "SHELL:-sERROR_ON_WASM_CHANGES_AFTER_LINK"
  "SHELL:-sWASM_BIGINT"
  "SHELL:-sNO_DISABLE_EXCEPTION_CATCHING"
  "SHELL:-sEXPORT_EXCEPTION_HANDLING_HELPERS"
)

# Option to copy example.html file to index.html in the binary folder
# In order to test the example:
# - go to the binary folder where f3d.js, f3d.wasm and f3d.data are located
# - run `python -m http.server 8000`
# - open a browser to http://localhost:8000
option(F3D_WASM_COPY_EXAMPLE "Copy HTML example to binary folder" ON)

if(F3D_WASM_COPY_EXAMPLE)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/example.html"
    "${CMAKE_BINARY_DIR}/bin/index.html"
    COPYONLY)
endif()
