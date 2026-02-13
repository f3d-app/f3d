## Tests that needs draco plugin
## This file is only added if draco is enabled
f3d_application_test(NAME TestDRACO DATA suzanne.drc PLUGINS draco)
f3d_application_test(NAME TestPipedDRACO DATA suzanne.drc PLUGINS draco ARGS --force-reader=Draco PIPED)
f3d_application_test(NAME TestDRACOColoring DATA suzanne.drc ARGS --scalar-coloring --coloring-component=0 PLUGINS draco)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10884
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240214)
  f3d_application_test(NAME TestGLTFDracoImporter DATA Box_draco.gltf PLUGINS draco ARGS --camera-position=-1.6,1.3,2.7)
  f3d_application_test(NAME TestGLTFDracoImporterWithoutCompression DATA BoxAnimated.gltf PLUGINS draco ARGS --animation-time=2 --animation-progress)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250923)
  f3d_application_test(NAME TestPipedGLTFDracoImporter DATA f3d.glb PLUGINS draco ARGS --force-reader=GLBDraco PIPED)
endif()

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/draco/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_application_test(NAME TestDefaultConfigFileDraco DATA suzanne.drc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugins;draco")
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/draco/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_application_test(NAME TestThumbnailConfigFileDraco DATA suzanne.drc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugins;draco")
endif()
