# Shared resources dir
set(f3d_resources_dir "share/f3d")
if (F3D_MACOS_BUNDLE)
  set(f3d_resources_dir "f3d.app/Contents/Resources")
endif()
set(f3d_configs_dir "${f3d_resources_dir}/configs")
if (UNIX AND NOT APPLE)
  if (NOT F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
    set(f3d_configs_dir "${CMAKE_INSTALL_FULL_SYSCONFDIR}/f3d")
  endif()
endif()

# Default config files
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/configs/"
  DESTINATION "${f3d_configs_dir}"
  COMPONENT configuration
  EXCLUDE_FROM_ALL)

# Other ressoure files
if(UNIX AND NOT APPLE AND NOT ANDROID)
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d.desktop"
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
elseif(WIN32)
  if (F3D_WINDOWS_INSTALL_LOGOS_FOR_NSIS_PACKAGING)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico"
      DESTINATION "${f3d_resources_dir}" COMPONENT assets)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logotype64.bmp"
      DESTINATION "${f3d_resources_dir}" COMPONENT assets)
  endif()
endif()
