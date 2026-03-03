## Tests that needs pdal plugin
## This file is only added if pdal is enabled
f3d_test(NAME TestLAZ DATA simple.laz ARGS --load-plugins=pdal --scalar-coloring --coloring-array=Color --coloring-component=-1)
f3d_test(NAME TestBPF DATA simple-extra.bpf ARGS --load-plugins=pdal --scalar-coloring --coloring-array=Color --coloring-component=-1)
f3d_test(NAME TestTerraScanBin DATA 20020715-time-color.bin ARGS --load-plugins=pdal --scalar-coloring --coloring-array=Color --coloring-component=-1)

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/pdal/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileLAZ DATA simple.laz CONFIG config_build LONG_TIMEOUT)
    f3d_test(NAME TestDefaultConfigFileBPF DATA simple-extra.bpf CONFIG config_build LONG_TIMEOUT)
    f3d_test(NAME TestDefaultConfigFileTerraScanBin DATA 20020715-time-color.bin CONFIG config_build LONG_TIMEOUT)
  endif()
  file(COPY "${F3D_SOURCE_DIR}/plugins/pdal/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileLAZ DATA simple.laz CONFIG thumbnail_build LONG_TIMEOUT)
  f3d_test(NAME TestThumbnailConfigFileBPF DATA simple-extra.bpf CONFIG thumbnail_build LONG_TIMEOUT)
  f3d_test(NAME TestThumbnailConfigFileTerraScanBin DATA 20020715-time-color.bin CONFIG thumbnail_build LONG_TIMEOUT)
endif()
