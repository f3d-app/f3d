# F3D Installation
if(F3D_BUILD_APPLICATION)
  install(TARGETS f3d
    EXPORT f3dTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT application
    BUNDLE DESTINATION "." COMPONENT bundle)
endif()

if(F3D_INSTALL_SDK OR BUILD_SHARED_LIBS)
  install(TARGETS libf3d
    EXPORT f3dTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT library
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME} COMPONENT headers)
endif()

# F3DShellExtension for Windows
if (F3D_BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
  install(TARGETS F3DShellExtension
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT shellext)
endif()

# Java bindings
if (F3D_BINDINGS_JAVA)
  install(TARGETS javaf3d
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT java)
  install_jar(f3d-jar DESTINATION "share/java" COMPONENT java)
endif()

# Python module
if (F3D_BINDINGS_PYTHON)
  if(WIN32)
    set(PYTHON_INSTALL_PATH ${CMAKE_INSTALL_BINDIR})
  else()
    set(PYTHON_INSTALL_PATH "${CMAKE_INSTALL_LIBDIR}/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
  endif()
  install(TARGETS pyf3d
    LIBRARY DESTINATION ${PYTHON_INSTALL_PATH} COMPONENT pythonmodule)
endif()

# Documentation
set(F3D_DOC_DIR ".")

if (UNIX AND NOT APPLE AND NOT ANDROID)
  set(F3D_DOC_DIR ${CMAKE_INSTALL_DOCDIR})
endif()

install(FILES LICENSE.md doc/THIRD_PARTY_LICENSES.md README.md
  DESTINATION ${F3D_DOC_DIR} COMPONENT documentation)

list(APPEND config_files
     "${CMAKE_SOURCE_DIR}/resources/config.json"
     "${CMAKE_SOURCE_DIR}/resources/thumbnail.json"
    )

# Default config file
if (UNIX AND NOT APPLE)
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
      install(FILES ${config_files}
        DESTINATION "share/f3d" COMPONENT configuration)
    else()
      message(STATUS "Enabling F3D_INSTALL_DEFAULT_CONFIGURATION_FILE, while not enabling "
              "F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX implies that installation of the config file rely on
              CMAKE_INSTALL_FULL_SYSCONFDIR and that the config file will not be packaged by cpack.")
      install(FILES ${config_files}
        DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/f3d" COMPONENT configuration)
    endif()
  endif()
endif()

# Get supported mime types
get_property(F3D_SUPPORTED_MIME_TYPES GLOBAL PROPERTY F3D_SUPPORTED_MIME_TYPES)

# Other ressoure files
if(UNIX AND NOT APPLE AND NOT ANDROID)
  configure_file(
    "${CMAKE_SOURCE_DIR}/resources/f3d.desktop.in"
    "${CMAKE_BINARY_DIR}/resources/f3d.desktop")

  install(FILES "${CMAKE_BINARY_DIR}/resources/f3d.desktop"
    DESTINATION "share/applications" COMPONENT assets)
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo32.png"
    DESTINATION "share/icons/hicolor/32x32/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo48.png"
    DESTINATION "share/icons/hicolor/48x48/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo64.png"
    DESTINATION "share/icons/hicolor/64x64/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo.svg"
    DESTINATION "share/icons/hicolor/scalable/apps" COMPONENT assets
    RENAME "f3d.svg")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/completion.bash"
    DESTINATION "share/bash-completion/completions" COMPONENT shellext
    RENAME "f3d")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/completion.zsh"
    DESTINATION "share/zsh/site-functions" COMPONENT shellext
    RENAME "_f3d")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/completion.fish"
    DESTINATION "share/fish/vendor_completions.d" COMPONENT shellext
    RENAME "f3d.fish")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/app.f3d.F3D.metainfo.xml"
    DESTINATION "share/metainfo" COMPONENT assets)
  if(TARGET man)
    install(FILES "${CMAKE_BINARY_DIR}/f3d.1.gz"
      DESTINATION "share/man/man1/" COMPONENT documentation)
  endif()
  if (F3D_INSTALL_THUMBNAILER_FILES)
    configure_file(
      "${CMAKE_SOURCE_DIR}/resources/f3d.thumbnailer.in"
      "${CMAKE_BINARY_DIR}/resources/f3d.thumbnailer")
    install(FILES "${CMAKE_BINARY_DIR}/resources/f3d.thumbnailer"
      DESTINATION "share/thumbnailers/" COMPONENT shellext)
  endif()
  if (F3D_INSTALL_MIME_TYPES_FILES)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-3d-formats.xml"
      DESTINATION "share/mime/packages" COMPONENT mimetypes
      RENAME "f3d-3d-formats.xml")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-3d-image-formats.xml"
      DESTINATION "share/mime/packages" COMPONENT mimetypes
      RENAME "f3d-3d-image-formats.xml")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-vtk-formats.xml"
      DESTINATION "share/mime/packages" COMPONENT mimetypes
      RENAME "f3d-vtk-formats.xml")
    if (F3D_PLUGIN_BUILD_EXODUS)
      install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-exodus-formats.xml"
        DESTINATION "share/mime/packages" COMPONENT mimetypes
        RENAME "f3d-3d-exodus-formats.xml")
    endif()
    if (F3D_PLUGIN_BUILD_OCCT)
      install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-occt-formats.xml"
        DESTINATION "share/mime/packages" COMPONENT mimetypes
        RENAME "f3d-occt-formats.xml")
    endif()
    if (F3D_PLUGIN_BUILD_ASSIMP)
      install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-assimp-formats.xml"
        DESTINATION "share/mime/packages" COMPONENT mimetypes
        RENAME "f3d-assimp-formats.xml")
    endif()
  endif()
elseif(WIN32)
  if (F3D_INSTALL_LOGOS_FOR_NSIS_PACKAGING)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/logo.ico"
      DESTINATION "." COMPONENT assets)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/logotype64.bmp"
      DESTINATION "." COMPONENT assets)
  endif()
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    install(FILES ${config_files}
      DESTINATION "." COMPONENT configuration)
  endif()
elseif(APPLE AND NOT F3D_MACOS_BUNDLE)
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    install(FILES ${config_files}
      DESTINATION "." COMPONENT configuration)
  endif()
endif()

# SDK
if(F3D_INSTALL_SDK)
  install(EXPORT f3dTargets
    NAMESPACE f3d::
    DESTINATION "lib/cmake/f3d"
    COMPONENT cmake)

  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    "cmake/f3dConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/f3dConfig.cmake"
    INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}")
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/f3dConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMinorVersion)

  install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/f3dConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/f3dConfigVersion.cmake"
      "${CMAKE_CURRENT_LIST_DIR}/f3dEmbed.cmake"
    DESTINATION
      "lib/cmake/${PROJECT_NAME}"
    COMPONENT cmake)
endif()
