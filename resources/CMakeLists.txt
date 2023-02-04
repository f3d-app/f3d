list(APPEND config_files
     "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
     "${CMAKE_CURRENT_SOURCE_DIR}/thumbnail.json"
    )

# Default config file
if (UNIX AND NOT APPLE)
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    if (F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
      install(FILES ${config_files}
        DESTINATION "share/f3d" COMPONENT configuration)
    else()
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
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/f3d.desktop")

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/f3d.desktop"
    DESTINATION "share/applications" COMPONENT assets)
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo32.png"
    DESTINATION "share/icons/hicolor/32x32/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo48.png"
    DESTINATION "share/icons/hicolor/48x48/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo64.png"
    DESTINATION "share/icons/hicolor/64x64/apps" COMPONENT assets
    RENAME "f3d.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo.svg"
    DESTINATION "share/icons/hicolor/scalable/apps" COMPONENT assets
    RENAME "f3d.svg")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/completion.bash"
    DESTINATION "share/bash-completion/completions" COMPONENT shellext
    RENAME "f3d")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/completion.zsh"
    DESTINATION "share/zsh/site-functions" COMPONENT shellext
    RENAME "_f3d")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/completion.fish"
    DESTINATION "share/fish/vendor_completions.d" COMPONENT shellext
    RENAME "f3d.fish")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/app.f3d.F3D.metainfo.xml"
    DESTINATION "share/metainfo" COMPONENT assets)
  if(TARGET man)
    install(FILES "${CMAKE_BINARY_DIR}/f3d.1.gz"
      DESTINATION "share/man/man1/" COMPONENT documentation)
  endif()
  if (F3D_LINUX_INSTALL_THUMBNAILER_FILES)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/f3d.thumbnailer.in"
      "${CMAKE_CURRENT_BINARY_DIR}/f3d.thumbnailer")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/f3d.thumbnailer"
      DESTINATION "share/thumbnailers/" COMPONENT shellext)
  endif()
elseif(WIN32)
  if (F3D_WINDOWS_INSTALL_LOGOS_FOR_NSIS_PACKAGING)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico"
      DESTINATION "./${F3D_WINDOWS_RESOURCES_INSTALL_DIR}" COMPONENT assets)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logotype64.bmp"
      DESTINATION "./${F3D_WINDOWS_RESOURCES_INSTALL_DIR}" COMPONENT assets)
  endif()
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    install(FILES ${config_files}
      DESTINATION "./${F3D_WINDOWS_RESOURCES_INSTALL_DIR}" COMPONENT configuration)
  endif()
elseif(APPLE AND NOT F3D_MACOS_BUNDLE)
  if (F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    install(FILES ${config_files}
      DESTINATION "." COMPONENT configuration)
  endif()
endif()
