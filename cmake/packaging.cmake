# F3D Packaging

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Kitware SAS")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_EXECUTABLES f3d f3d)

if(WIN32 AND NOT UNIX)
  set(f3d_url "https://gitlab.kitware.com/f3d/f3d")
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
  set(CPACK_NSIS_URL_INFO_ABOUT ${f3d_url})
else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_GENERATOR TGZ TXZ)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/logo.icns")
  endif()
endif()

include(CPack)
