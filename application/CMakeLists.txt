function(f3d_create_exec target_name)
  # Define f3d target
  add_executable(${target_name} ${F3D_SOURCE_FILES})
  target_link_libraries(${target_name} PUBLIC libf3d)

  if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(${target_name} PRIVATE Threads::Threads)
  endif()

  if(F3D_LINUX_LINK_FILESYSTEM)
    target_link_libraries(${target_name} PRIVATE stdc++fs)
  endif()

  set_target_properties(${target_name} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    CXX_STANDARD 17
    )

  if (APPLE)
    if (F3D_MACOS_BUNDLE)
      set_target_properties(${target_name} PROPERTIES INSTALL_RPATH @loader_path/../../../${CMAKE_INSTALL_LIBDIR})
    else ()
      set_target_properties(${target_name} PROPERTIES INSTALL_RPATH @loader_path/../${CMAKE_INSTALL_LIBDIR})
    endif ()
  elseif (UNIX)
    set_target_properties(${target_name} PROPERTIES INSTALL_RPATH $ORIGIN/../${CMAKE_INSTALL_LIBDIR})
  elseif (MSVC)
    set_property(DIRECTORY ${F3D_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target_name}) # Set ${target_name} as startup project in visual studio
    set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$<TARGET_FILE_DIR:VTK::CommonCore>") # Add VTK in MSVC PATH environment variable
  endif ()

  target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

  if (F3D_USE_EXTERNAL_CXXOPTS)
    target_link_libraries(${target_name} PRIVATE cxxopts::cxxopts)
  else ()
    target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/cxxopts>)
  endif ()

  if (F3D_USE_EXTERNAL_NLOHMANN_JSON)
    target_link_libraries(${target_name} PRIVATE nlohmann_json::nlohmann_json)
  else ()
    target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/nlohmann_json>)
  endif ()

  if (F3D_MODULE_DMON)
    if (F3D_USE_EXTERNAL_DMON)
      target_link_libraries(${target_name} PRIVATE dmon::dmon)
    else ()
      target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/dmon>)
    endif ()
  endif ()

  target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/tinyfiledialogs>)

  set(f3d_compile_options_private "")
  set(f3d_compile_options_public "")
  set(f3d_link_options_public "")

  # This is required to avoid cxxopts
  # splitting input positional option by commas
  target_compile_definitions(${target_name} PRIVATE "CXXOPTS_VECTOR_DELIMITER='\\0'")

  # macOS bundle
  if (F3D_MACOS_BUNDLE)
    target_compile_definitions(${target_name} PRIVATE F3D_MACOS_BUNDLE)
  endif ()

  # raytracing
  if (F3D_MODULE_RAYTRACING)
    target_compile_definitions(${target_name} PRIVATE F3D_MODULE_RAYTRACING)
  endif ()

  # exr
  if (F3D_MODULE_EXR)
    target_compile_definitions(${target_name} PRIVATE F3D_MODULE_EXR)
  endif ()

  # dmon
  if (F3D_MODULE_DMON)
    target_compile_definitions(${target_name} PRIVATE F3D_MODULE_DMON)
  endif ()

  # tinyfiledialogs
  if (F3D_MODULE_TINYFILEDIALOGS)
    target_compile_definitions(${target_name} PRIVATE F3D_MODULE_TINYFILEDIALOGS)
  endif ()

  if(VTK_VERSION VERSION_LESS 9.3.20240914)
    # Headless EGL build
    if (VTK_OPENGL_HAS_EGL)
      target_compile_definitions(${target_name} PRIVATE F3D_HEADLESS_BUILD)
    endif ()
  endif ()

  # F3D_STRICT_BUILD
  list(APPEND f3d_compile_options_private ${f3d_strict_build_compile_options})
  if(F3D_STRICT_BUILD)
    if(MSVC)
      target_compile_definitions(${target_name} PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
    elseif(APPLE)
      target_compile_definitions(${target_name} PRIVATE GL_SILENCE_DEPRECATION)
    endif()
  endif()

  # F3D_TESTING_DISABLE_CATCH_ALL
  if (F3D_TESTING_DISABLE_CATCH_ALL)
    target_compile_definitions(${target_name} PRIVATE F3D_TESTING_DISABLE_CATCH_ALL)
  endif ()

  # Coverage
  list(APPEND f3d_compile_options_public ${f3d_coverage_compile_options})
  list(APPEND f3d_link_options_public ${f3d_coverage_link_options})

  # Sanitizer
  list(APPEND f3d_compile_options_public ${f3d_sanitizer_compile_options})
  list(APPEND f3d_link_options_public ${f3d_sanitizer_link_options})

  # MacOS Bundle app
  if(F3D_MACOS_BUNDLE)
    set_target_properties(${target_name} PROPERTIES MACOSX_BUNDLE TRUE)

    # Add logo icon
    set(MACOSX_BUNDLE_ICON_FILE logo.icns)
    set(f3d_ICON ${F3D_SOURCE_DIR}/resources/logo.icns)
    set_source_files_properties(${f3d_ICON} PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${target_name} PRIVATE ${f3d_ICON})

    # Set other MacOS vars
    set(MACOSX_BUNDLE_EXECUTABLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_INFO_STRING ${PROJECT_DESCRIPTION})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "app.f3d.F3D")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${F3D_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${F3D_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${F3D_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "Michael Migliore, Mathieu Westphal")

    # Generate MacOS bundle using CMake variable generated by plugins
    get_property(F3D_MACOS_BUNDLE_XML GLOBAL PROPERTY F3D_MACOS_BUNDLE_XML)
    configure_file("${F3D_SOURCE_DIR}/resources/BundleInfo.plist.in"
      "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
    set_target_properties(${target_name} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
  endif()

  if(APPLE)
    target_link_libraries(${target_name} PRIVATE "-framework Cocoa")
  endif()

  # Windows executable without console
  if(WIN32)
    set_target_properties(${target_name} PROPERTIES WIN32_EXECUTABLE TRUE)
  endif()

  target_compile_options(${target_name} PUBLIC ${f3d_compile_options_public} PRIVATE ${f3d_compile_options_private})
  target_link_options(${target_name} PUBLIC ${f3d_link_options_public})
endfunction()

# Generate F3DIcon buffer
include("f3dEmbed")

f3d_embed_file(
  INPUT "${F3D_SOURCE_DIR}/resources/logo32.png"
  NAME F3DIcon
  BINARY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/F3DConfig.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/F3DConfig.h")

set(F3D_SOURCE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DColorMapTools.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DConfigFileTools.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DOptionsTools.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DPluginsTools.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DStarter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DSystemTools.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cxx
)

if(WIN32)
  list(APPEND F3D_SOURCE_FILES "${F3D_SOURCE_DIR}/resources/f3d.rc")
endif()

if(APPLE)
  list(APPEND F3D_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm")
endif()

# Add source for dialogs
list(APPEND F3D_SOURCE_FILES "${F3D_SOURCE_DIR}/external/tinyfiledialogs/tinyfiledialogs.cxx")

# Create the f3d application
f3d_create_exec(f3d)

# Create f3d-console on Windows
if(WIN32 AND F3D_WINDOWS_BUILD_CONSOLE_APPLICATION)
  f3d_create_exec(f3d-console)
  # Overwrite console property
  set_target_properties(f3d-console PROPERTIES WIN32_EXECUTABLE FALSE)
endif()

# Create man
if(UNIX AND NOT APPLE)
  option(F3D_LINUX_GENERATE_MAN "Generate and install a man entry for f3d using help2man" OFF)
  mark_as_advanced(F3D_LINUX_GENERATE_MAN)
  if(F3D_LINUX_GENERATE_MAN)
    find_program(HELP2MAN help2man)
    find_program(GZIP gzip)
    if(HELP2MAN AND GZIP)
      set(MAN_OUTPUT_FILE ${CMAKE_BINARY_DIR}/f3d.1)
      add_custom_command(
        OUTPUT ${MAN_OUTPUT_FILE}
        COMMAND ${HELP2MAN} $<TARGET_FILE:f3d> -N -n "fast and minimalist 3D viewer" > ${MAN_OUTPUT_FILE}
        COMMAND ${GZIP} -fn ${MAN_OUTPUT_FILE}
        DEPENDS f3d)
      add_custom_target(man ALL DEPENDS ${MAN_OUTPUT_FILE})

      install(FILES "${CMAKE_BINARY_DIR}/f3d.1.gz"
        DESTINATION "share/man/man1/" COMPONENT documentation)
    else()
      message(FATAL_ERROR "help2man or gzip not found, cannot create man entry, please disable F3D_LINUX_GENERATE_MAN or install them")
    endif()
  endif()
endif()

# Testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# Installing
install(TARGETS f3d
  EXPORT f3dTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT application
  BUNDLE DESTINATION "." COMPONENT application)

# The targets and config file are only installed with the sdk
install(EXPORT f3dTargets
  NAMESPACE f3d::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
  COMPONENT sdk
  EXCLUDE_FROM_ALL)

install(
  FILES
    "${F3D_SOURCE_DIR}/cmake/application-config.cmake"
  DESTINATION
    "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
  COMPONENT sdk
  EXCLUDE_FROM_ALL)

# Default config files
install(
  DIRECTORY "${F3D_SOURCE_DIR}/resources/configs/"
  DESTINATION "${f3d_config_dir}"
  COMPONENT configuration
  EXCLUDE_FROM_ALL)

# Default color maps
install(
  DIRECTORY "${F3D_SOURCE_DIR}/resources/colormaps/"
  DESTINATION "${f3d_resources_dir}/colormaps"
  COMPONENT colormaps
  EXCLUDE_FROM_ALL)

# Other ressoure files
if(UNIX AND NOT APPLE AND NOT ANDROID)
  install(FILES "${F3D_SOURCE_DIR}/resources/f3d.desktop"
    DESTINATION "share/applications" COMPONENT assets)
  install(FILES "${F3D_SOURCE_DIR}/resources/logo16.png"
    DESTINATION "share/icons/hicolor/16x16/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo24.png"
    DESTINATION "share/icons/hicolor/24x24/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo32.png"
    DESTINATION "share/icons/hicolor/32x32/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo48.png"
    DESTINATION "share/icons/hicolor/48x48/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo64.png"
    DESTINATION "share/icons/hicolor/64x64/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo256.png"
    DESTINATION "share/icons/hicolor/256x256/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo.svg"
    DESTINATION "share/icons/hicolor/scalable/apps" COMPONENT assets
    RENAME "f3d.svg")
  install(FILES "${F3D_SOURCE_DIR}/resources/logo-mono.svg"
    DESTINATION "share/icons/HighContrast/scalable/apps" COMPONENT assets
    RENAME "f3d.svg")
  install(FILES "${F3D_SOURCE_DIR}/resources/completion.bash"
    DESTINATION "share/bash-completion/completions" COMPONENT shellext
    RENAME "f3d")
  install(FILES "${F3D_SOURCE_DIR}/resources/completion.zsh"
    DESTINATION "share/zsh/site-functions" COMPONENT shellext
    RENAME "_f3d")
  install(FILES "${F3D_SOURCE_DIR}/resources/completion.fish"
    DESTINATION "share/fish/vendor_completions.d" COMPONENT shellext
    RENAME "f3d.fish")
  install(FILES "${F3D_SOURCE_DIR}/resources/app.f3d.F3D.metainfo.xml"
    DESTINATION "share/metainfo" COMPONENT assets)
endif()
