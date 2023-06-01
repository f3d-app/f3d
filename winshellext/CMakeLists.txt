add_library(F3DShellExtension SHARED
  F3DShellExtension.cxx
  F3DShellExtensionClassFactory.h
  F3DThumbnailProvider.h
  F3DThumbnailProvider.cxx
  RegistryHelpers.h
  F3DShellExtension.def)

target_compile_definitions(F3DShellExtension PUBLIC UNICODE F3D_WINDOWS_THUMBNAIL_TIMEOUT=8000)

target_include_directories(F3DShellExtension PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
if (F3D_USE_EXTERNAL_NLOHMANN_JSON)
  target_link_libraries(F3DShellExtension PRIVATE nlohmann_json::nlohmann_json)
else ()
  target_include_directories(F3DShellExtension PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/nlohmann_json>)
endif ()

target_link_libraries(F3DShellExtension PUBLIC libf3d PRIVATE pathcch shlwapi windowscodecs)

set_target_properties(F3DShellExtension PROPERTIES
  CXX_STANDARD 17
)

# Installing
install(TARGETS F3DShellExtension
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT shellext)
