## Tests that needs webifc plugin
## This file is only added if webifc is enabled
f3d_application_test(NAME TestWebIFC DATA IfcOpenHouse_IFC4.ifc PLUGIN webifc)
f3d_application_test(NAME TestWebIFCDefines DATA IfcOpenHouse_IFC4.ifc PLUGIN webifc ARGS -DIFC.circle_segments=24 -DIFC.read_openings=0 -DIFC.read_spaces=0)
f3d_application_test(NAME TestPipedWebIFC DATA IfcOpenHouse_IFC4.ifc PLUGIN webifc ARGS --force-reader=IFC PIPED)

if(NOT F3D_MACOS_BUNDLE AND VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  file(COPY "${F3D_SOURCE_DIR}/plugins/webifc/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  f3d_application_test(NAME TestDefaultConfigFileWebIFC DATA IfcOpenHouse_IFC4.ifc CONFIG config_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;webifc")
  file(COPY "${F3D_SOURCE_DIR}/plugins/webifc/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_application_test(NAME TestThumbnailConfigFileWebIFC DATA IfcOpenHouse_IFC4.ifc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;webifc")
endif()
