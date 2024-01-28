option(F3D_PLUGIN_BUILD_ALEMBIC "Alembic plugin (ABC files)" OFF)
option(F3D_PLUGIN_BUILD_ASSIMP "Assimp plugin (FBX, OFF, DAE, DXF, X and 3MF files)" OFF)
option(F3D_PLUGIN_BUILD_DRACO "Draco plugin (DRC files)" OFF)
option(F3D_PLUGIN_BUILD_EXODUS "ExodusII plugin (EX2 files)" ON)
option(F3D_PLUGIN_BUILD_OCCT "OpenCASCADE plugin (STEP and IGES files)" OFF)
option(F3D_PLUGIN_BUILD_USD "Universal Scene Description plugin (USD files)" OFF)
option(F3D_PLUGIN_BUILD_VDB "VDB plugin, using OpenVDB (VDB files)" OFF)

add_subdirectory(native)

if (F3D_PLUGIN_BUILD_ALEMBIC)
  add_subdirectory(alembic)
endif()

if (F3D_PLUGIN_BUILD_ASSIMP)
  add_subdirectory(assimp)
endif()

if (F3D_PLUGIN_BUILD_DRACO)
  add_subdirectory(draco)
endif()

if (F3D_PLUGIN_BUILD_EXODUS)
  add_subdirectory(exodus)
endif()

if (F3D_PLUGIN_BUILD_OCCT)
  add_subdirectory(occt)
endif()

if (F3D_PLUGIN_BUILD_USD)
  add_subdirectory(usd)
endif()

if (F3D_PLUGIN_BUILD_VDB)
  # VDB plugin needs vtkOpenVDBReader: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9363
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220714)
    add_subdirectory(vdb)
  else ()
    message(FATAL_ERROR "VDB plugin requires VTK >= 9.2.20220714")
  endif()
endif()
