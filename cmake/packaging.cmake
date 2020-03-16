# F3D Packaging

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY  "${PROJECT_NAME}")
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
  set(CPACK_NSIS_INSTALLED_ICON_NAME "logo.ico")
  set(CPACK_NSIS_URL_INFO_ABOUT ${f3d_url})
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_DISPLAY_NAME "F3D")

  # Include the scripts
  # FileAssociation.nsh, from https://nsis.sourceforge.io/File_Association, has to be installed in NSIS\Include
  set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
   "!include \\\"${PATH_TO_PLUGINS}\\\\FileFunc.nsh\\\"\n\
   !include \\\"${PATH_TO_PLUGINS}\\\\FileAssociation.nsh\\\"")

  # Create association on install
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
    "\\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vtk' 'VTK Legacy Data Format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vtp' 'VTK XML PolyData Format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vtu' 'VTK XML UnstructuredGrid Format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vti' 'VTK XML ImageData format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vtr' 'VTK XML RectilinearGrid format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vts' 'VTK XML StructuredGrid format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.vtm' 'VTK XML Multiblock Format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.ply' 'Polygon File format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.stl' 'Standard Triangle Language format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.dcm' 'DICOM file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.nrrd' '\\\"nearly raw raster data\\\" file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.nhrd' '\\\"nearly raw raster data\\\" file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.mhd' 'MetaHeader MetaIO file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.mha' 'MetaHeader MetaIO file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.obj' 'Wavefront OBJ file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.gltf' 'GL Transmission Format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.glb' 'GL Transmission Format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.3ds' 'Autodesk 3DS Max file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.wrl' 'VRML file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.gml' 'CityGML file format'\n\
    \\\${RegisterExtension} '$INSTDIR\\\\bin\\\\f3d.exe' '.pts' 'Point cloud file format'\n\
    \\\${RefreshShellIcons}")

  # Remove association on uninstall
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
    "\\\${UnRegisterExtension} '.vtk' 'VTK Legacy Data Format'\n\
    \\\${UnRegisterExtension} '.vtp' 'VTK XML PolyData Format'\n\
    \\\${UnRegisterExtension} '.vtu' 'VTK XML UnstructuredGrid Format'\n\
    \\\${UnRegisterExtension} '.vti' 'VTK XML ImageData format'\n\
    \\\${UnRegisterExtension} '.vtr' 'VTK XML RectilinearGrid format'\n\
    \\\${UnRegisterExtension} '.vts' 'VTK XML StructuredGrid format'\n\
    \\\${UnRegisterExtension} '.vtm' 'VTK XML Multiblock Format'\n\
    \\\${UnRegisterExtension} '.ply' 'Polygon File format'\n\
    \\\${UnRegisterExtension} '.stl' 'Standard Triangle Language format'\n\
    \\\${UnRegisterExtension} '.dcm' 'DICOM file format'\n\
    \\\${UnRegisterExtension} '.nrrd' '\\\"nearly raw raster data\\\" file format'\n\
    \\\${UnRegisterExtension} '.nhrd' '\\\"nearly raw raster data\\\" file format'\n\
    \\\${UnRegisterExtension} '.mhd' 'MetaHeader MetaIO file format'\n\
    \\\${UnRegisterExtension} '.mha' 'MetaHeader MetaIO file format'\n\
    \\\${UnRegisterExtension} '.obj' 'Wavefront OBJ file format'\n\
    \\\${UnRegisterExtension} '.gltf' 'GL Transmission Format'\n\
    \\\${UnRegisterExtension} '.glb' 'GL Transmission Format'\n\
    \\\${UnRegisterExtension} '.3ds' 'Autodesk 3DS Max file format'\n\
    \\\${UnRegisterExtension} '.wrl' 'VRML file format'\n\
    \\\${UnRegisterExtension} '.gml' 'CityGML file format'\n\
    \\\${UnRegisterExtension} '.pts' 'Point Cloud file format'\n\
    \\\${RefreshShellIcons}")

else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_GENERATOR TGZ TXZ)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/logo.icns")
  endif()
endif()

include(CPack)
