## Tests that needs vdb plugin
## This file is only added if vdb is enabled

set (_outOfRangeDoubleStr "\
12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012\
34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234\
56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456\
78901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678\
90123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\
1234567890123456789012345678901234567890\
")

f3d_application_test(NAME TestVDBVolume DATA icosahedron.vdb ARGS --load-plugins=vdb --volume --volume-inverse LABELS "vdb")
f3d_application_test(NAME TestVDBVerbose DATA icosahedron.vdb ARGS --load-plugins=vdb REGEXP "PartitionedDataSet" NO_RENDER LABELS "vdb")
f3d_application_test(NAME TestVDBDefinesDownsamplingFactor DATA icosahedron.vdb ARGS --load-plugins=vdb -DVDB.downsampling_factor=0.2 --volume --volume-inverse LABELS "vdb")
f3d_application_test(NAME TestVDBDefinesInexistent DATA icosahedron.vdb ARGS --load-plugins=vdb -Dvdb.downsampling_factor=0.2 REGEXP "did you mean 'VDB.downsampling_factor'" NO_BASELINE LABELS "vdb")
f3d_application_test(NAME TestVDBDefinesDownsamplingFactorParseError DATA icosahedron.vdb ARGS --load-plugins=vdb -DVDB.downsampling_factor=abcde --verbose REGEXP "Could not parse VDB.downsampling_factor" NO_BASELINE LABELS "vdb")
f3d_application_test(NAME TestVDBDefinesDownsamplingFactorOutOfRangeError DATA icosahedron.vdb ARGS --load-plugins=vdb -DVDB.downsampling_factor=${_outOfRangeDoubleStr} --verbose REGEXP "VDB.downsampling_factor out of range" NO_BASELINE LABELS "vdb")
f3d_application_test(NAME TestVDBCommandScriptReaderOptions SCRIPT DATA icosahedron.vdb ARGS --load-plugins=vdb --volume --volume-inverse LABELS "vdb") # set_reader_option VDB.downsampling_factor 0.2; reload_current_file_group

# Test point sprites requires VTK > 9.3
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  f3d_application_test(NAME TestVDBPoints DATA sphere_points.vdb ARGS --load-plugins=vdb -o LABELS "vdb")
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  f3d_application_test(NAME TestPipedVDBVolume DATA icosahedron.vdb ARGS --load-plugins=vdb --volume --volume-inverse --force-reader=VDB PIPED LABELS "vdb")
endif()

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/vdb/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_application_test(NAME TestDefaultConfigFileVDB DATA icosahedron.vdb CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "vdb")
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/vdb/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_application_test(NAME TestThumbnailConfigFileVDB DATA icosahedron.vdb CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "vdb")
endif()
