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
  include(tests.pdal.cmake)
endif()

include(tests.watch.cmake)

# Some documentation images require VTK >= 9.6 to be generated correctly.
# While we could generate some of them with VTK < 9.6,
# the goal here is not feature coverage but to ensure that the documentation is correct.
if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.0 AND F3D_TESTING_ENABLE_ILLUSTRATION_TESTS)
  include(tests.documentation.cmake)
endif()
