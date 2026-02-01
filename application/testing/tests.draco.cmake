## Tests that needs draco plugin
## This file is only added if draco is enabled
f3d_test(NAME TestDRACO DATA suzanne.drc ARGS --load-plugins=draco)
f3d_test(NAME TestPipedDRACO DATA suzanne.drc ARGS --load-plugins=draco --force-reader=Draco PIPED)
f3d_test(NAME TestDRACOColoring DATA suzanne.drc ARGS --scalar-coloring --coloring-component=0 --load-plugins=draco)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10884
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240214)
  f3d_test(NAME TestGLTFDracoImporter DATA Box_draco.gltf ARGS --load-plugins=draco --camera-position=-1.6,1.3,2.7)
  f3d_test(NAME TestGLTFDracoImporterWithoutCompression DATA BoxAnimated.gltf ARGS --load-plugins=draco --animation-time=2 --animation-progress)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250923)
  f3d_test(NAME TestPipedGLTFDracoImporter DATA f3d.glb ARGS --load-plugins=draco --force-reader=GLBDraco PIPED)
endif()  

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/draco/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileDraco DATA suzanne.drc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/draco/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileDraco DATA suzanne.drc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
endif()
