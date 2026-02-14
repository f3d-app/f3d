include(f3dTest)

include(tests.native.cmake)
include(tests.features.cmake)
include(tests.backends.cmake)
include(tests.custom.cmake)
include(tests.home.cmake)
include(tests.screenshot.cmake)
include(tests.gaussian.cmake)
include(tests.interaction.cmake)
include(tests.plugins.cmake)

if(F3D_PLUGIN_BUILD_ALEMBIC)
  include(tests.alembic.cmake)
endif()

if(F3D_PLUGIN_BUILD_ASSIMP)
  include(tests.assimp.cmake)
endif()

if(F3D_PLUGIN_BUILD_DRACO)
  include(tests.draco.cmake)
endif()

if(F3D_PLUGIN_BUILD_HDF)
  include(tests.hdf.cmake)
endif()

if(F3D_PLUGIN_BUILD_WEBIFC)
  include(tests.webifc.cmake)
endif()

if(F3D_PLUGIN_BUILD_OCCT)
  include(tests.occt.cmake)
endif()

if(F3D_PLUGIN_BUILD_USD)
  include(tests.usd.cmake)
endif()

if(F3D_PLUGIN_BUILD_VDB)
  include(tests.vdb.cmake)
endif()

if(F3D_PLUGIN_BUILD_PDAL)
  f3d_test(NAME TestE57 DATA bunnyInt19.e57 ENV "LD_LIBRARY_PATH=${F3D_SOURCE_DIR}/testing/data:/__w/f3d/f3d/dependencies/install/lib" ARGS --load-plugins=pdal)
endif()

include(tests.watch.cmake)
