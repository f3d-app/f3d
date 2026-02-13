## Generic plugin tests
## May require specific plugins to be enabled

# Test plugin fail code path
f3d_application_test(NAME TestPluginVerbose ARGS --verbose REGEXP "Loading plugin \"native\"" NO_BASELINE LABELS "plugin;native")
f3d_application_test(NAME TestPluginNonExistent PLUGINS dummy REGEXP "Plugin failed to load" NO_BASELINE)
if(WIN32)
  set(_TEST_PLUGIN_INVALID_REGEXP "is not a valid Win32 application")
elseif(APPLE)
  set(_TEST_PLUGIN_INVALID_REGEXP "not.*mach-o file")
else()
  set(_TEST_PLUGIN_INVALID_REGEXP "file too short")
endif()
f3d_application_test(NAME TestPluginInvalid ARGS --load-plugins=${F3D_SOURCE_DIR}/testing/data/invalid.so REGEXP ${_TEST_PLUGIN_INVALID_REGEXP} NO_BASELINE LABELS "plugin")

if(UNIX AND NOT APPLE)
  f3d_application_test(NAME TestPluginInvalidSystem ARGS --verbose --load-plugins=invalid REGEXP "file too short" ENV "LD_LIBRARY_PATH=${F3D_SOURCE_DIR}/testing/data" NO_BASELINE LABELS "plugin")
endif()

if(WIN32)
  set(_dirname "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
  set(_dirname "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif()

if(BUILD_SHARED_LIBS)
  f3d_application_test(NAME TestPluginNoInit ARGS --verbose --load-plugins=${_dirname}/${CMAKE_SHARED_LIBRARY_PREFIX}f3d${CMAKE_SHARED_LIBRARY_SUFFIX} NO_BASELINE REGEXP "Cannot find init_plugin symbol in library" LABELS "plugin")
endif()

if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_application_test(NAME TestMultiplePluginsLoad DATA cow.vtp ARGS --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load" LABELS "plugin;assimp;alembic")
endif()


# Test multi plugin list-readers
if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_application_test(NAME TestReadersListMultiplePlugins ARGS --list-readers --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load" LABELS "plugin;assimp;alembic")
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_application_test(NAME TestForceReaderFail DATA suzanne.stl ARGS --force-reader=GLTF NO_BASELINE REGEXP "Some of these files could not be loaded: failed to load scene" LABELS "plugin")
  f3d_application_test(NAME TestForceReaderPass DATA suzanne.not_supported ARGS --force-reader=STL LABELS "plugin")
  f3d_application_test(NAME TestForceReaderInvalid DATA suzanne.stl ARGS --force-reader=INVALID NO_BASELINE REGEXP "Forced reader .* doesn't exist" LABELS "plugin")

  if(F3D_PLUGIN_BUILD_DRACO)
    f3d_application_test(NAME TestForceReaderGLTFDraco DATA BoxAnimated.gltf PLUGINS draco ARGS --force-reader=GLTFDraco LABELS "plugin")
    f3d_application_test(NAME TestForceReaderGLTFDracoIntoGLTF DATA Box_draco.gltf PLUGINS draco ARGS --force-reader=GLTF NO_BASELINE REGEXP "failed to load scene" LABELS "plugin")
  endif()
endif()

# Test scan plugins
if(NOT F3D_MACOS_BUNDLE)
  f3d_application_test(NAME TestScanPluginsCheckNative ARGS --scan-plugins NO_RENDER NO_BASELINE REGEXP " - native" LABELS "plugin")

  if(F3D_PLUGIN_BUILD_HDF)
    f3d_application_test(NAME TestScanPluginsCheckHDF ARGS --scan-plugins NO_RENDER NO_BASELINE REGEXP " - hdf" LABELS "plugin")
  endif()
endif()
