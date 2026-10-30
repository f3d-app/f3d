set(classes
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::vtkextUSD
  FORCE_STATIC
  CLASSES ${classes}
  SOURCES F3DUSDMemoryResolver.cxx
  HEADERS F3DUSDMemoryResolver.h
)

list(GET PXR_LIBRARIES 0 _first_lib)
get_target_property(_target_type ${_first_lib} TYPE)
if(_target_type STREQUAL "STATIC_LIBRARY")
  # When using WHOLE_ARCHIVE, we need to remove system libraries on Windows to avoid linking issues
  # shlwapi.lib(SHLWAPI.dll) : error LNK2005: __NULL_IMPORT_DESCRIPTOR already defined in dbghelp.lib(dbghelp.dll)
  set(PXR_WHOLE_ARCHIVE_LIBRARIES ${PXR_LIBRARIES})
  if(WIN32)
    list(REMOVE_ITEM PXR_WHOLE_ARCHIVE_LIBRARIES Shlwapi.lib Dbghelp.lib)
  endif()
  vtk_module_link(f3d::vtkextUSD PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,${PXR_WHOLE_ARCHIVE_LIBRARIES}>)
else()
  vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})
endif()

vtk_module_include(f3d::vtkextUSD PRIVATE ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkextUSD CXX_STANDARD 20)
