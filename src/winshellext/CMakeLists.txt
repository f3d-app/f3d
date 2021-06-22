add_library(F3DShellExtension SHARED
  F3DShellExtension.cxx
  F3DShellExtensionClassFactory.h
  F3DThumbnailProvider.h
  F3DThumbnailProvider.cxx
  RegistryHelpers.h
  F3DShellExtension.def)

target_compile_definitions(F3DShellExtension PUBLIC UNICODE)

target_include_directories(F3DShellExtension PUBLIC ${CMAKE_BINARY_DIR})

target_link_libraries(F3DShellExtension PUBLIC libf3d PRIVATE pathcch shlwapi)

set_target_properties(F3DShellExtension PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
