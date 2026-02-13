## Tests that needs occt plugin
## This file is only added if occt is enabled
f3d_application_test(NAME TestSTEP DATA f3d.stp PLUGINS occt ARGS --up=+Z)
f3d_application_test(NAME TestIGES DATA f3d.igs PLUGINS occt ARGS --up=+Z)
f3d_application_test(NAME TestBREP DATA f3d.brep PLUGINS occt ARGS --up=+Z)
f3d_application_test(NAME TestBinaryBREP DATA f3d.bin.brep PLUGINS occt ARGS --up=+Z)
f3d_application_test(NAME TestSTEPDefines DATA cheese.stp PLUGINS occt ARGS -DSTEP.linear_deflection=0.5 -DSTEP.angular_deflection=0.9 -DSTEP.relative_deflection=1)
f3d_application_test(NAME TestIGESDefines DATA cheese.igs PLUGINS occt ARGS -DIGES.read_wire=0 --line-width=5 --up=+Z)
f3d_application_test(NAME TestBREPDefines DATA cheese.brep PLUGINS occt ARGS -DBREP.linear_deflection=100 --up=+Z)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_application_test(NAME TestInvalidSTEP DATA invalid.step PLUGINS occt ARGS --verbose REGEXP "failed to load scene" NO_BASELINE)
  f3d_application_test(NAME TestInvalidBREP DATA invalid.brep PLUGINS occt ARGS --verbose REGEXP "failed to load scene" NO_BASELINE)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251223)
  f3d_application_test(NAME TestPipedSTEP DATA f3d.stp PLUGINS occt ARGS --up=+Z --force-reader=STEP PIPED)
  f3d_application_test(NAME TestPipedBREP DATA f3d.brep PLUGINS occt ARGS --up=+Z --force-reader=BREP PIPED)
  f3d_application_test(NAME TestPipedBinaryBREP DATA f3d.bin.brep PLUGINS occt ARGS --up=+Z --force-reader=BREP PIPED)
endif()

if(F3D_PLUGIN_OCCT_COLORING_SUPPORT)
  f3d_application_test(NAME TestXBF DATA f3d.xbf PLUGINS occt ARGS --up=+Z)
  f3d_application_test(NAME TestXBFDefines DATA cheese.xbf PLUGINS occt ARGS -DXBF.angular_deflection=1 -DXBF.relative_deflection=1 --up=+Z)
  f3d_application_test(NAME TestXCAFColors DATA xcaf-colors.stp PLUGINS occt ARGS -csy --up=+Z --line-width=3 --camera-direction=-1,-1,-1)
  f3d_application_test(NAME TestXCAFColorsXBF DATA xcaf-colors.xbf PLUGINS occt ARGS -csy --up=+Z --line-width=3 --camera-direction=-1,-1,-1)
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
    f3d_application_test(NAME TestInvalidXBF DATA invalid.xbf PLUGINS occt ARGS --verbose REGEXP "failed to load scene" NO_BASELINE)
  endif()
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251223)
    f3d_application_test(NAME TestPipedXBF DATA f3d.xbf PLUGINS occt ARGS --up=+Z --force-reader=XBF PIPED)
  endif()

  if (NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/occt/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
      f3d_application_test(NAME TestDefaultConfigFileOCCT DATA f3d.stp CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;occt")
    endif()

    file(COPY "${F3D_SOURCE_DIR}/plugins/occt/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_application_test(NAME TestThumbnailConfigFileOCCT DATA f3d.stp CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;occt")
  endif()
endif()
