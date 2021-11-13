# F3D Installation

# F3D
install(TARGETS f3d
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ".")

get_target_property(LIBF3D_TYPE libf3d TYPE)
if(LIBF3D_TYPE STREQUAL "SHARED_LIBRARY")
  install(TARGETS libf3d
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# F3DShellExtension for Windows
if (BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
  install(TARGETS F3DShellExtension
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# Documentation
set(F3D_DOC_DIR ".")

if (UNIX AND NOT APPLE)
  set(F3D_DOC_DIR ${CMAKE_INSTALL_DOCDIR})
endif()

install(FILES LICENSE README.md
  DESTINATION ${F3D_DOC_DIR})

# Default config file
if (UNIX AND NOT APPLE)
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
      install(FILES "${CMAKE_SOURCE_DIR}/resources/config.json"
        DESTINATION ".")
    else()
      if (NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
        message(WARNING "Enabling F3D_INSTALL_DEFAULT_CONFIGURATION_FILE, while not installing to /usr. "
                "Default configuration file may be installed in a location that will not be scanned by F3D. "
                "You can set F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX to ensure the file will be scanned.")
      endif()
      install(FILES "${CMAKE_SOURCE_DIR}/resources/config.json"
        DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/f3d")
    endif()
  endif()
endif()

# Other ressoure files
if(UNIX AND NOT APPLE)
  install(FILES "${CMAKE_SOURCE_DIR}/resources/f3d.desktop"
    DESTINATION "share/applications")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo32.png"
    DESTINATION "share/icons/hicolor/32x32/apps"
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo48.png"
    DESTINATION "share/icons/hicolor/48x48/apps"
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo64.png"
    DESTINATION "share/icons/hicolor/64x64/apps"
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo.svg"
    DESTINATION "share/icons/hicolor/scalable/apps"
    RENAME "f3d.svg")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/completion.bash"
    DESTINATION "share/bash-completion/completions"
    RENAME "f3d")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/completion.zsh"
    DESTINATION "share/zsh/site-functions"
    RENAME "_f3d")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/completion.fish"
    DESTINATION "share/fish/vendor_completions.d"
    RENAME "f3d.fish")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/com.kitware.F3D.metainfo.xml"
    DESTINATION "share/metainfo")
  if(TARGET man)
    install(FILES "${CMAKE_BINARY_DIR}/f3d.1.gz"
      DESTINATION "share/man/man1/")
  endif()
  if (F3D_INSTALL_THUMBNAILER_FILES)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/f3d.thumbnailer"
      DESTINATION "share/thumbnailers/")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/f3d_z.thumbnailer"
      DESTINATION "share/thumbnailers/")
  endif()
  if (F3D_INSTALL_MIME_TYPES_FILE)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-3d-formats.xml"
      DESTINATION "share/mime/packages"
      RENAME "f3d-3d-formats.xml")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-3d-image-formats.xml"
      DESTINATION "share/mime/packages"
      RENAME "f3d-3d-image-formats.xml")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-3d-misc-formats.xml"
      DESTINATION "share/mime/packages"
      RENAME "f3d-3d-misc-formats.xml")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-vtk-formats.xml"
      DESTINATION "share/mime/packages"
      RENAME "f3d-vtk-formats.xml")
    if (F3D_MODULE_OCCT)
      install(FILES "${CMAKE_SOURCE_DIR}/resources/mime-types-cad-formats.xml"
        DESTINATION "share/mime/packages"
        RENAME "f3d-cad-formats.xml")
    endif()
  endif()
elseif(WIN32 AND NOT UNIX)
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo.ico"
    DESTINATION ".")
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/config.json"
      DESTINATION ".")
  endif()
elseif(APPLE AND NOT F3D_MACOS_BUNDLE)
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/config.json"
      DESTINATION ".")
  endif()
endif()
