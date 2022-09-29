# F3D Packaging

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY  "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}-app")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.md")
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
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources\\\\logotype64.bmp")
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
   !include \\\"FileFunc.nsh\\\"\n\
   !include \\\"FileAssociation.nsh\\\"")

  # Retrieve file association
  get_property(F3D_FILE_ASSOCIATION_NSIS GLOBAL PROPERTY F3D_SUPPORTED_EXTENSIONS)
  list(TRANSFORM F3D_FILE_ASSOCIATION_NSIS REPLACE "^(.+)$" "'.\\1' '\\1 file'")

  # Create association on install
  set(F3D_REGISTER_LIST "${F3D_FILE_ASSOCIATION_NSIS}")
  list(TRANSFORM F3D_REGISTER_LIST PREPEND "\\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' ")
  list(JOIN F3D_REGISTER_LIST "\n      " F3D_REGISTER_STRING)
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    StrCmp $REGISTER_EXTENSIONS \\\"0\\\" doNotRegisterExtensions
      ${F3D_REGISTER_STRING}
      \\\${RefreshShellIcons}
    doNotRegisterExtensions:\n\n")

  if (F3D_BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
      ; Register shell extension
      StrCmp $REGISTER_THUMBNAILS \\\"0\\\" doNotRegisterThumbnails
        ExecWait '\\\"$SYSDIR\\\\regsvr32.exe\\\" /s \\\"$INSTDIR\\\\bin\\\\F3DShellExtension.dll\\\"'
      doNotRegisterThumbnails:\n")
  else()
    set(CPACK_NSIS_INSTALL_OPTIONS_PAGE_COMMANDS "
      ; Disable thumbnail
      !insertmacro MUI_INSTALLOPTIONS_WRITE \\\"NSIS.InstallOptions.ini\\\" \\\"Field 7\\\" \\\"Flags\\\" \\\"DISABLED\\\"\n\
      !insertmacro MUI_INSTALLOPTIONS_WRITE \\\"NSIS.InstallOptions.ini\\\" \\\"Field 7\\\" \\\"State\\\" \\\"0\\\"\n")
  endif()

  # Remove association on uninstall
  set(F3D_UNREGISTER_LIST "${F3D_FILE_ASSOCIATION_NSIS}")
  list(TRANSFORM F3D_UNREGISTER_LIST PREPEND "\\\${UnRegisterExtension} ")
  list(JOIN F3D_UNREGISTER_LIST "\n      " F3D_UNREGISTER_STRING)
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    StrCmp $REGISTER_EXTENSIONS \\\"0\\\" doNotUnregisterExtensions
      ${F3D_UNREGISTER_STRING}
      \\\${RefreshShellIcons}
    doNotUnregisterExtensions:\n\n")

  if (F3D_BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
      ; Unregister shell extension
      StrCmp $REGISTER_THUMBNAILS \\\"0\\\" doNotUnregisterThumbnails
        ExecWait '\\\"$SYSDIR\\\\regsvr32.exe\\\" /s /u \\\"$INSTDIR\\\\bin\\\\F3DShellExtension.dll\\\"'
      doNotUnregisterThumbnails:\n")
  endif()

else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Whisley <whisley.santos@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libbsd0, libxdmcp6, libglvnd0, libxcb1, libc6, libgcc1, libstdc++6, libopengl0, libglx0, libx11-6")
    set(CPACK_GENERATOR TGZ TXZ DEB)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/logo.icns")
  endif()
endif()

# Never package "pythonmodule" component
# The SDK ("cmake" and "headers" components) is not packaged for macOS bundle
# "vtkext" component must be packaged if libf3d is static and not a macOS bundle
set(CPACK_COMPONENTS_ALL assets configuration documentation shellext mimetypes)
if(F3D_MACOS_BUNDLE)
  list(APPEND CPACK_COMPONENTS_ALL bundle)
else()
  list(APPEND CPACK_COMPONENTS_ALL application library cmake headers)
  if(NOT BUILD_SHARED_LIBS)
    list(APPEND CPACK_COMPONENTS_ALL vtkext)
  endif()
endif()

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
include(CPack)
