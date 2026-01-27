## Tests that needs ifc plugin
## This file is only added if ifc is enabled
f3d_test(NAME TestIFC DATA ifc/IfcOpenHouse_IFC4.ifc ARGS --load-plugins=ifc)
f3d_test(NAME TestIFCDefines DATA ifc/IfcOpenHouse_IFC4.ifc ARGS --load-plugins=ifc -DIFC.circle_segments=24 -DIFC.read_openings=0 -DIFC.read_spaces=0)
f3d_test(NAME TestPipedIFC DATA ifc/IfcOpenHouse_IFC4.ifc ARGS --load-plugins=ifc --force-reader=IFC PIPED)

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/ifc/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileIFC DATA ifc/IfcOpenHouse_IFC4.ifc CONFIG config_build LONG_TIMEOUT TONE_MAPPING)
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/ifc/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestThumbnailConfigFileIFC DATA ifc/IfcOpenHouse_IFC4.ifc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()
endif()
