# Generate F3DIcon buffer
include("${CMAKE_SOURCE_DIR}/cmake/f3dEmbed.cmake")

f3d_embed_file(
  INPUT "${CMAKE_SOURCE_DIR}/resources/logo32.png"
  NAME F3DIcon
  BINARY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/F3DConfig.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/F3DConfig.h")

set(F3D_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DConfigFileTools.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DOptionsParser.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DStarter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cxx
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

if(UNIX AND NOT APPLE)
  option(F3D_APPLICATION_LINK_FILESYSTEM "Link with libstdc++fs" ON)
  mark_as_advanced(F3D_APPLICATION_LINK_FILESYSTEM)
  if(F3D_APPLICATION_LINK_FILESYSTEM)
    target_link_libraries(f3d PRIVATE stdc++fs)
  endif()
endif()

set_target_properties(f3d PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  CXX_VISIBILITY_PRESET hidden
  )

if (APPLE)
  set_target_properties(f3d PROPERTIES INSTALL_RPATH @loader_path/../lib)
elseif (UNIX)
  set_target_properties(f3d PROPERTIES INSTALL_RPATH $ORIGIN/../${CMAKE_INSTALL_LIBDIR})
endif ()

target_compile_features(f3d PRIVATE cxx_std_17)

target_include_directories(f3d
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )

set(f3d_compile_options_private "")
set(f3d_compile_options_public "")
set(f3d_link_options_public "")

# F3D_STRICT_BUILD
list(APPEND f3d_compile_options_private ${f3d_strict_build_compile_options})
if(F3D_STRICT_BUILD)
  if(MSVC)
    target_compile_definitions(f3d PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
  elseif(APPLE)
    target_compile_definitions(f3d PRIVATE GL_SILENCE_DEPRECATION)
  endif()
endif()

# Coverage
list(APPEND f3d_compile_options_public ${f3d_coverage_compile_options})
list(APPEND f3d_link_options_public ${f3d_coverage_link_options})

# Sanitizer
list(APPEND f3d_compile_options_public ${f3d_sanitizer_compile_options})
list(APPEND f3d_link_options_public ${f3d_sanitizer_link_options})

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

  # Set other MacOS vars
  set(MACOSX_BUNDLE_EXECUTABLE_NAME ${PROJECT_NAME})
  set(MACOSX_BUNDLE_INFO_STRING ${PROJECT_DESCRIPTION})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "app.f3d.F3D")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_COPYRIGHT "Michael Migliore, Mathieu Westphal")

  # Generate MacOS bundle
  configure_file("${CMAKE_SOURCE_DIR}/resources/BundleInfo.plist.in"
    "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
  set_target_properties(f3d PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
endif()

if(APPLE)
  target_link_libraries(f3d PRIVATE "-framework Cocoa")
endif()

# Windows executable without console
if(WIN32)
  if(F3D_WINDOWS_GUI)
    set_target_properties(f3d PROPERTIES WIN32_EXECUTABLE TRUE)
  else()
    # force usage of wWinMainCRTStartup in console mode for Unicode support
    list(APPEND f3d_link_options_public "/ENTRY:wWinMainCRTStartup")
  endif()
endif()

target_compile_options(f3d PUBLIC ${f3d_compile_options_public} PRIVATE ${f3d_compile_options_private})
target_link_options(f3d PUBLIC ${f3d_link_options_public})

# Create man
option(F3D_GENERATE_MAN "Generate and install a man entry for f3d using help2man" OFF)
mark_as_advanced(F3D_GENERATE_MAN)
if(F3D_GENERATE_MAN)
  find_program(HELP2MAN help2man)
  find_program(GZIP gzip)
  if(HELP2MAN AND GZIP)
    set(MAN_OUTPUT_FILE ${CMAKE_BINARY_DIR}/f3d.1)
    add_custom_command(
      OUTPUT ${MAN_OUTPUT_FILE}
      COMMAND ${HELP2MAN} $<TARGET_FILE:f3d> -N -n "fast and minimalist 3D viewer" > ${MAN_OUTPUT_FILE}
      COMMAND ${GZIP} -f ${MAN_OUTPUT_FILE}
      DEPENDS f3d)
    add_custom_target(man ALL DEPENDS ${MAN_OUTPUT_FILE})
  else()
    message(WARNING "help2man or gzip not found, cannot create man entry")
  endif()
endif()
