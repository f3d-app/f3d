set(classes
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::vtkextUSD
  FORCE_STATIC
  CLASSES ${classes}
  SOURCES F3DUSDMemoryResolver.cxx
  HEADERS F3DUSDMemoryResolver.h
)

vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})

list(GET PXR_LIBRARIES 0 _first_lib)
get_target_property(_target_type ${_first_lib} TYPE)
if(_target_type STREQUAL "STATIC_LIBRARY")
  # The following ensures that certain USD symbols are linked correctly is USD is built as a static library.
  # However, that will only work if `PXR_ENABLE_NAMESPACES` is disabled.
  if (WIN32)
    foreach (_usd_symbols "??_R0?AV?$VtArray@Vhalf@pxr_half@@@@@8" "??_R3SdfUsdzFileFormat@@8")
      vtk_module_link_options(f3d::vtkextUSD INTERFACE "/INCLUDE:${_usd_symbols}")
    endforeach ()
  else ()
    foreach (_usd_symbols _ZTIN8pxr_half4halfE SdfUsdzFileFormatTokens)
      if (APPLE)
        vtk_module_link_options(f3d::vtkextUSD INTERFACE "-Wl,-u,_${_usd_symbols}")
      elseif(UNIX)
        vtk_module_link_options(f3d::vtkextUSD INTERFACE "-Wl,--undefined=${_usd_symbols}")
      endif()
    endforeach()
  endif()
endif()

vtk_module_include(f3d::vtkextUSD PRIVATE ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkextUSD CXX_STANDARD 20)
