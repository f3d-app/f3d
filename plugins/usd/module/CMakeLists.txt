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
  if(APPLE)
    # on macOS, linking specific symbols doesn't work yet and result in runtime exception:
    # Class PlugNotice::DidRegisterPlugins (derived from TfNotice) is undefined in the TfType system
    vtk_module_link(f3d::vtkextUSD PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,${PXR_LIBRARIES}>)
  else()
    vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})

    # The following ensures that certain USD symbols are linked correctly is USD is built as a static library.
    # However, that will only work if `PXR_ENABLE_NAMESPACES` is disabled.
    message(NOTICE "Linking USD static libraries with specific symbols, make sure PXR_ENABLE_NAMESPACES is disabled")
    if(WIN32)
      # demangled symbols to include:
      # - class VtArray<class pxr_half::half> `RTTI Type Descriptor'
      # - SdfUsdzFileFormat::`RTTI Class Hierarchy Descriptor'
      foreach (_usd_symbols "??_R0?AV?$VtArray@Vhalf@pxr_half@@@@@8" "??_R3SdfUsdzFileFormat@@8")
        vtk_module_link_options(f3d::vtkextUSD INTERFACE "/INCLUDE:${_usd_symbols}")
      endforeach ()
    else()
      # demangled symbols to include:
      # - typeinfo for pxr_half::half
      # - SdfUsdzFileFormatTokens
      foreach (_usd_symbols _ZTIN8pxr_half4halfE SdfUsdzFileFormatTokens)
        vtk_module_link_options(f3d::vtkextUSD INTERFACE "-Wl,--undefined=${_usd_symbols}")
      endforeach()
    endif()
  endif()
else()
  vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})
endif()

vtk_module_include(f3d::vtkextUSD PRIVATE ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkextUSD CXX_STANDARD 20)
