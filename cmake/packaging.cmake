# F3D Packaging

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY  "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}-app")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_EXECUTABLES f3d f3d)
set(CPACK_CREATE_DESKTOP_LINKS f3d)

set(CPACK_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(WIN32 AND NOT UNIX)
  set(f3d_url "https://github.com/f3d-app/f3d")
  set(f3d_ico "${CMAKE_SOURCE_DIR}/resources/logo.ico")
  set(CPACK_GENERATOR NSIS64 ZIP)
  # For some reason, we need Windows backslashes
  # https://www.howtobuildsoftware.com/index.php/how-do/PNb/cmake-nsis-bmp-cpack-how-to-set-an-icon-in-nsis-install-cmake
  # BMP3 format is also required (recommended size is 150x57)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources\\\\logo.bmp")
  set(CPACK_NSIS_MENU_LINKS ${f3d_url} "F3D Website")
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_MUI_ICON ${f3d_ico})
  set(CPACK_NSIS_MUI_UNIICON ${f3d_ico})
  set(CPACK_NSIS_INSTALLED_ICON_NAME "logo.ico")
  set(CPACK_NSIS_URL_INFO_ABOUT ${f3d_url})
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_DISPLAY_NAME "F3D")

  set(CPACK_NSIS_EXTRA_INCLUDE_DIR "${CMAKE_BINARY_DIR}")

  # Include the scripts
  # FileAssociation.nsh, from https://nsis.sourceforge.io/File_Association, has to be installed in NSIS\Include
  set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
   "\
   !include \\\"${PATH_TO_PLUGINS}\\\\FileFunc.nsh\\\"\n\
   !include \\\"${PATH_TO_PLUGINS}\\\\FileAssociation.nsh\\\"")

  # Create association on install
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "\
    StrCmp $REGISTER_EXTENSIONS \\\"0\\\" doNotRegisterExtensions\n\
      !include \\\"NSIS.RegisterCommands.nsh\\\"\n\
      \\\${RefreshShellIcons}\n\
    doNotRegisterExtensions:\n\n")

  if (BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
    list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "\
      StrCmp $REGISTER_THUMBNAILS \\\"0\\\" doNotRegisterThumbnails\n\
        ExecWait '\\\"$SYSDIR\\\\regsvr32.exe\\\" /s \\\"$INSTDIR\\\\bin\\\\F3DShellExtension.dll\\\"'\n\
      doNotRegisterThumbnails:\n")
  else()
    set(CPACK_NSIS_INSTALL_OPTIONS_PAGE_COMMANDS "\n\
      ; Disable thumbnail
      !insertmacro MUI_INSTALLOPTIONS_WRITE \\\"NSIS.InstallOptions.ini\\\" \\\"Field 7\\\" \\\"Flags\\\" \\\"DISABLED\\\"\n\
      !insertmacro MUI_INSTALLOPTIONS_WRITE \\\"NSIS.InstallOptions.ini\\\" \\\"Field 7\\\" \\\"State\\\" \\\"0\\\"\n")
  endif()

  # Remove association on uninstall
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "\
    StrCmp $REGISTER_EXTENSIONS \\\"0\\\" doNotUnregisterExtensions\n\
      !include \\\"NSIS.UnregisterCommands.nsh\\\"\n\
      \\\${RefreshShellIcons}\n\
    doNotUnregisterExtensions:\n\n")

  if (BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
    list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "\
      StrCmp $REGISTER_THUMBNAILS \\\"0\\\" doNotUnregisterThumbnails\n\
        ExecWait '\\\"$SYSDIR\\\\regsvr32.exe\\\" /s /u \\\"$INSTDIR\\\\bin\\\\F3DShellExtension.dll\\\"'\n\
      doNotUnregisterThumbnails:\n")
  endif()

else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_GENERATOR TGZ TXZ DEB)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/logo.icns")
  endif()
endif()

include(CPack)
