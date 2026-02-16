## Tests that needs hdf plugin
## Contains specific features tests
## This file is only added if hdf is enabled

f3d_test(NAME TestExodus DATA disk_out_ref.ex2 ARGS --load-plugins=hdf -s --camera-position=-11,-2,-49)
f3d_test(NAME TestExodusExo DATA box.exo ARGS --load-plugins=hdf NO_RENDER NO_BASELINE REGEXP "Number of points: 24")
f3d_test(NAME TestExodusG DATA box.g ARGS --load-plugins=hdf NO_RENDER NO_BASELINE REGEXP "Number of points: 24")
f3d_test(NAME TestExodusE DATA single_timestep.e ARGS --load-plugins=hdf NO_RENDER NO_BASELINE REGEXP "Number of points: 1331")
f3d_test(NAME TestExodusConfig DATA disk_out_ref.ex2 CONFIG ${F3D_SOURCE_DIR}/testing/configs/exodus.json ARGS -s --camera-position=-11,-2,-49)
f3d_test(NAME TestNetCDF DATA temperature_grid.nc ARGS --load-plugins=hdf -s)

if (VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
  f3d_test(NAME TestVTKHDF DATA blob.vtkhdf ARGS --load-plugins=hdf -s)
  f3d_test(NAME TestAMRDataSet DATA amr.vtkhdf ARGS --load-plugins=hdf -s)
endif()

if (VTK_VERSION VERSION_GREATER_EQUAL 9.4.0)
  f3d_test(NAME TestVTKHDFPartitionedDataSetCollection DATA pdc_sphere_cone.vtkhdf ARGS --load-plugins=hdf -s)
endif()

if (VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251109)
  f3d_test(NAME TestPipedVTKHDF DATA blob.vtkhdf ARGS --load-plugins=hdf -s PIPED VTKHDF)
endif()

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/hdf/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileExodus DATA disk_out_ref.ex2 CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/hdf/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileExodus DATA disk_out_ref.ex2 CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)

  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileVTKHDF DATA blob.vtkhdf CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()
  if (VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
    f3d_test(NAME TestThumbnailConfigFileVTKHDF DATA blob.vtkhdf CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()

  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileNetCDF DATA temperature_grid.nc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()
  f3d_test(NAME TestThumbnailConfigFileNetCDF DATA temperature_grid.nc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
endif()

## Feature tests that rely on HDF plugin

# Test Generic Importer Verbose animation. Regex contains the time range.
f3d_test(NAME TestVerboseGenericImporterAnimation DATA small.ex2 ARGS --load-plugins=hdf --verbose NO_BASELINE REGEXP "0, 0.00429999")

# Test animation with generic importer and coloring
f3d_test(NAME TestAnimationGenericImporter DATA small.ex2 ARGS -sb --load-plugins=hdf --animation-time=0.003 --animation-progress)

# Test animation with generic importer, coloring and point sprites
f3d_test(NAME TestAnimationGenericImporterPointSprites DATA small.ex2 ARGS -sbo --load-plugins=hdf --animation-time=0.003 --animation-progress)

# Test animation with generic importer, coloring and a custom scalar range
f3d_test(NAME TestAnimationGenericImporterScalarRange DATA small.ex2 ARGS -sb --load-plugins=hdf --animation-time=0.003 --animation-progress --coloring-range=0,1e7)

# Test Generic Importer Verbose animation with a single frame.
f3d_test(NAME TestVerboseAnimationSingleTimestep DATA single_timestep.e ARGS --load-plugins=hdf --verbose NO_BASELINE REGEXP "0, 0")

# Test no render animation time. Regex contains a part of the range of the VEL_ field.
f3d_test(NAME TestNoRenderAnimation DATA small.ex2 ARGS  --load-plugins=hdf --animation-time=0.003 REGEXP "-994.473, 33.9259" NO_RENDER)

# Test animation time clamping
f3d_test(NAME TestAnimationTimeLimitsHigh DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=10)
f3d_test(NAME TestAnimationTimeLimitsLow DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=-5)
f3d_test(NAME TestAnimationTimeLimitsHighNoWarning DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=0.0043001 REGEXP_FAIL "outside of range" NO_RENDER)
f3d_test(NAME TestAnimationTimeLimitsLowNoWarning DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=-0.000001 REGEXP_FAIL "outside of range" NO_RENDER)

# Test if negative range is respected when loading a file without specifying the animation time
f3d_test(NAME TestTimeRangeLessThanZeroNoAnimationTime DATA negative_range_animated.e ARGS -s --load-plugins=hdf)

# Test if the animation-time works when set to zero and time range[0] is less than zero
f3d_test(NAME TestTimeRangeLessThanZeroWithAnimationTime DATA negative_range_animated.e ARGS -s --load-plugins=hdf --animation-time=0)

# Test if a negative animation-time works when time range[0] is less than zero
f3d_test(NAME TestTimeRangeLessThanZeroNegativeAnimationTime DATA negative_range_animated.e ARGS -s --load-plugins=hdf --animation-time=-3)

f3d_test(NAME TestMultiFileAnimationUniqueUnique DATA negative_range_animated.e small.ex2 ARGS --animation-time=0.0043001 --animation-indices=0,1 --multi-file-mode=all --load-plugins=hdf )
if (VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
  f3d_test(NAME TestMultiFileAnimationMultiUnique DATA f3d.glb blob.vtkhdf ARGS --animation-time=2 --animation-indices=0,1 --multi-file-mode=all --opacity=0.5 --load-plugins=hdf )
endif()

if (NOT F3D_PLUGINS_STATIC_BUILD AND BUILD_SHARED_LIBS)

  # Test --load-plugins with the name of a dynamic plugin
  f3d_test(NAME TestPluginName DATA disk_out_ref.ex2 ARGS --load-plugins=hdf --verbose NO_BASELINE REGEXP "Loaded plugin hdf from")

  # Test --load-plugins with a full path plugin
  f3d_test(NAME TestPluginFullPath DATA disk_out_ref.ex2 ARGS --verbose --load-plugins "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_MODULE_PREFIX}f3d-plugin-hdf${CMAKE_SHARED_MODULE_SUFFIX}" NO_BASELINE REGEXP "Loaded plugin hdf from")
endif()
