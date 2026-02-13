## Tests that needs alembic plugin
## This file is only added if alembic is enabled
f3d_application_test(NAME TestAlembic DATA suzanne.abc ARGS -s PLUGIN alembic)
f3d_application_test(NAME TestAlembicNonFaceVarying DATA tetrahedron_non_facevarying_uv.abc ARGS -s PLUGIN alembic)
f3d_application_test(NAME TestAlembicAnimation DATA drop.abc PLUGIN alembic ARGS --animation-time=2 --animation-progress)
f3d_application_test(NAME TestAlembicAnimationXForm DATA xform_anim.abc PLUGIN alembic ARGS -g --animation-time=1.5 --animation-progress)
f3d_application_test(NAME TestAlembicAnimationXFormRotation DATA joint1.abc PLUGIN alembic ARGS -g --animation-time=1.5 --animation-progress)
f3d_application_test(NAME TestAlembicCurves DATA monkey_curves.abc PLUGIN alembic THRESHOLD 0.07) #  High threshold because of line rendering

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_application_test(NAME TestAlembicInvalid DATA invalid.abc PLUGIN alembic ARGS -s REGEXP "failed to load scene" NO_BASELINE)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  f3d_application_test(NAME TestPipedAlembic DATA suzanne.abc PLUGIN alembic ARGS -s --force-reader=Alembic PIPED)
endif()

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/alembic/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_application_test(NAME TestDefaultConfigFileAlembic DATA suzanne.abc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;alembic")
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/alembic/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_application_test(NAME TestThumbnailConfigFileAlembic DATA suzanne.abc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;alembic")
endif()
