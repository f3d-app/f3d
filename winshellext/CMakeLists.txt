set(F3D_WINDOWS_THUMBNAIL_TIMEOUT 8000 CACHE STRING "Windows thumbnail creation timeout in milliseconds")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/F3DThumbnailConfig.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/F3DThumbnailConfig.h"
  @ONLY)

# Retrieve extensions
get_property(F3D_SUPPORTED_EXTENSIONS_INITIALIZER_LIST GLOBAL PROPERTY F3D_SUPPORTED_EXTENSIONS)

list(TRANSFORM F3D_SUPPORTED_EXTENSIONS_INITIALIZER_LIST PREPEND "L\".")
list(TRANSFORM F3D_SUPPORTED_EXTENSIONS_INITIALIZER_LIST APPEND "\"")
list(JOIN F3D_SUPPORTED_EXTENSIONS_INITIALIZER_LIST ", " F3D_SUPPORTED_EXTENSIONS_INITIALIZER_LIST)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/F3DShellExtension.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/F3DShellExtension.cxx"
  @ONLY)

add_library(F3DShellExtension SHARED
  ${CMAKE_CURRENT_BINARY_DIR}/F3DShellExtension.cxx
  F3DShellExtensionClassFactory.h
  F3DThumbnailProvider.h
  F3DThumbnailProvider.cxx
  RegistryHelpers.h
  F3DShellExtension.def)

target_compile_definitions(F3DShellExtension PUBLIC UNICODE)

target_include_directories(F3DShellExtension PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(F3DShellExtension PUBLIC libf3d PRIVATE pathcch shlwapi windowscodecs)

set_target_properties(F3DShellExtension PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
