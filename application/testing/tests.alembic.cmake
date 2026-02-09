## Tests that needs alembic plugin
## This file is only added if alembic is enabled
f3d_test(NAME TestAlembic DATA suzanne.abc ARGS -s --load-plugins=alembic)
f3d_test(NAME TestAlembicNonFaceVarying DATA tetrahedron_non_facevarying_uv.abc ARGS -s --load-plugins=alembic)
f3d_test(NAME TestAlembicAnimation DATA drop.abc ARGS --animation-time=2 --load-plugins=alembic --animation-progress)
f3d_test(NAME TestAlembicAnimationXForm DATA xform_anim.abc ARGS -g --animation-time=1.5 --load-plugins=alembic --animation-progress)
f3d_test(NAME TestAlembicAnimationXFormRotation DATA joint1.abc ARGS -g --animation-time=1.5 --load-plugins=alembic --animation-progress)
f3d_test(NAME TestAlembicCurves DATA monkey_curves.abc ARGS --load-plugins=alembic THRESHOLD 0.07) #  High threshold because of line rendering

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_test(NAME TestAlembicInvalid DATA invalid.abc ARGS -s --load-plugins=alembic REGEXP "failed to load scene" NO_BASELINE)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  f3d_test(NAME TestPipedAlembic DATA suzanne.abc ARGS -s --load-plugins=alembic PIPED)
endif()

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/alembic/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileAlembic DATA suzanne.abc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/alembic/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileAlembic DATA suzanne.abc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
endif()
