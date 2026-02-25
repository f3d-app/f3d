## Generic plugin tests
## May require specific plugins to be enabled

# Test plugin fail code path
f3d_test(NAME TestPluginVerbose ARGS --verbose REGEXP "Loading plugin \"native\"" NO_BASELINE)
f3d_test(NAME TestPluginNonExistent ARGS --load-plugins=dummy REGEXP "Plugin failed to load" NO_BASELINE)
if(WIN32)
  set(_TEST_PLUGIN_INVALID_REGEXP "is not a valid Win32 application")
elseif(APPLE)
  set(_TEST_PLUGIN_INVALID_REGEXP "not.*mach-o file")
else()
  set(_TEST_PLUGIN_INVALID_REGEXP "file too short")
endif()
f3d_test(NAME TestPluginInvalid ARGS --load-plugins=${F3D_SOURCE_DIR}/testing/data/invalid.so REGEXP ${_TEST_PLUGIN_INVALID_REGEXP} NO_BASELINE)

if(UNIX AND NOT APPLE)
  f3d_test(NAME TestPluginInvalidSystem ARGS --verbose --load-plugins=invalid REGEXP "file too short" ENV "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${F3D_SOURCE_DIR}/testing/data" NO_BASELINE)
endif()

if(WIN32)
  set(_dirname "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
  set(_dirname "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif()

if(BUILD_SHARED_LIBS)
  f3d_test(NAME TestPluginNoInit ARGS --verbose --load-plugins=${_dirname}/${CMAKE_SHARED_LIBRARY_PREFIX}f3d${CMAKE_SHARED_LIBRARY_SUFFIX} NO_BASELINE REGEXP "Cannot find init_plugin symbol in library")
endif()

if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestMultiplePluginsLoad DATA cow.vtp ARGS --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load")
endif()


# Test multi plugin list-readers
if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestReadersListMultiplePlugins ARGS --list-readers --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load")
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_test(NAME TestForceReaderFail DATA suzanne.stl ARGS --force-reader=GLTF NO_BASELINE REGEXP "Some of these files could not be loaded: failed to load scene")
  f3d_test(NAME TestForceReaderPass DATA suzanne.not_supported ARGS --force-reader=STL)
  f3d_test(NAME TestForceReaderInvalid DATA suzanne.stl ARGS --force-reader=INVALID NO_BASELINE REGEXP "Forced reader .* doesn't exist")

  if(F3D_PLUGIN_BUILD_DRACO)
    f3d_test(NAME TestForceReaderGLTFDraco DATA BoxAnimated.gltf ARGS --load-plugins=draco --force-reader=GLTFDraco)
    f3d_test(NAME TestForceReaderGLTFDracoIntoGLTF DATA Box_draco.glb ARGS --load-plugins=draco --force-reader=GLTF NO_BASELINE REGEXP "failed to load scene")
  endif()
endif()

# Test scan plugins
if(NOT F3D_MACOS_BUNDLE)
  f3d_test(NAME TestScanPluginsCheckNative ARGS --scan-plugins NO_RENDER NO_BASELINE REGEXP " - native")

  if(F3D_PLUGIN_BUILD_HDF)
    f3d_test(NAME TestScanPluginsCheckHDF ARGS --scan-plugins NO_RENDER NO_BASELINE REGEXP " - hdf")
  endif()
endif()
