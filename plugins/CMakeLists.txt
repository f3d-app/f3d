option(F3D_PLUGIN_BUILD_EXODUS "ExodusII plugin (EX2 files)" ON)
option(F3D_PLUGIN_BUILD_OCCT "OpenCASCADE plugin (STEP and IGES files)" OFF)
option(F3D_PLUGIN_BUILD_ASSIMP "Assimp plugin (FBX, OFF, DAE anf DXF files)" OFF)
option(F3D_PLUGIN_BUILD_ALEMBIC "Alembic plugin (ABC files)" OFF)

add_subdirectory(native)

if (F3D_PLUGIN_BUILD_EXODUS)
  add_subdirectory(exodus)
endif()

if (F3D_PLUGIN_BUILD_OCCT)
  add_subdirectory(occt)
endif()

if (F3D_PLUGIN_BUILD_ASSIMP)
  add_subdirectory(assimp)
endif()

if (F3D_PLUGIN_BUILD_ALEMBIC)
  add_subdirectory(alembic)
endif()
