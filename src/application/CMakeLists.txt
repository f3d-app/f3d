# Generate F3DIcon buffer
include("${CMAKE_SOURCE_DIR}/cmake/f3dEmbed.cmake")

f3d_embed_file(
  INPUT "${CMAKE_SOURCE_DIR}/resources/logo32.png"
  NAME F3DIcon
  BINARY)

set(F3D_SOURCE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DStarter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DOptionsParser.cxx
)

if(WIN32)
  list(APPEND F3D_SOURCE_FILES "${CMAKE_SOURCE_DIR}/resources/f3d.rc")
endif()

if(APPLE)
  list(APPEND F3D_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm")
endif()

# Define f3d target
add_executable(f3d ${F3D_SOURCE_FILES})
target_link_libraries(f3d PUBLIC libf3d)

if (UNIX)
  target_link_libraries(f3d PRIVATE stdc++fs)
endif ()

set_target_properties(f3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if (APPLE)
  set_target_properties(f3d PROPERTIES INSTALL_RPATH @loader_path/../lib)
elseif (UNIX)
  set_target_properties(f3d PROPERTIES INSTALL_RPATH $ORIGIN/../lib)
endif ()

# Cannot upgrade to C++17 here, because <codecvt> header is deprecated and produces warnings on Windows
target_compile_features(f3d PRIVATE cxx_std_17)

target_include_directories(f3d
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )

if(F3D_STRICT_BUILD)
  if(MSVC)
    target_compile_options(f3d PRIVATE /W4 /WX)
    target_compile_definitions(f3d PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
  else()
    target_compile_options(f3d PRIVATE -Wall -Wextra -Wshadow -Werror)
  endif()
endif()

# MacOS Bundle app
if(F3D_MACOS_BUNDLE)
  set_target_properties(f3d PROPERTIES MACOSX_BUNDLE TRUE)

  # Add default configuration
  if(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    set(f3d_CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json)
    set_source_files_properties(${f3d_CONFIG} PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(f3d PRIVATE ${f3d_CONFIG})
  endif()

  # Add logo icon
  set(MACOSX_BUNDLE_ICON_FILE logo.icns)
  set(f3d_ICON ${CMAKE_SOURCE_DIR}/resources/logo.icns)
  set_source_files_properties(${f3d_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")
  target_sources(f3d PRIVATE ${f3d_ICON})
  configure_file("${CMAKE_SOURCE_DIR}/resources/BundleInfo.plist.in"
    "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
  set_target_properties(f3d PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
endif()

# Windows executable without console
if(WIN32)
  if(F3D_WINDOWS_GUI)
    set_target_properties(f3d PROPERTIES WIN32_EXECUTABLE TRUE)
  else()
    # force usage of wWinMainCRTStartup in console mode for Unicode support
    target_link_options(f3d PUBLIC "/ENTRY:wWinMainCRTStartup")
  endif()
endif()
