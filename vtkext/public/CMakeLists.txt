vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

# Although we have a single module
# we need to follow the full VTK module logic here
vtk_module_scan(
  MODULE_FILES      ${vtk_module_files}
  PROVIDES_MODULES  modules
  WANT_BY_DEFAULT   ON
  ENABLE_TESTS      ${BUILD_TESTING})

set(f3d_vtkext_headers_exclude "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240312)
  set(f3d_vtkext_headers_exclude "HEADERS_EXCLUDE_FROM_ALL;ON")
endif()
  
set(export_name "")
set(headers_component "")
set(f3d_vtk_no_install "")
set(f3d_vtk_force_static "")
get_target_property(f3d_vtk_target_type VTK::CommonCore TYPE)
if(BUILD_SHARED_LIBS AND NOT f3d_vtk_target_type STREQUAL STATIC_LIBRARY)
  # The headers and all "dev" part are installed as part of the plugin SDK
  # The library itself is installed as part of the library
  set(export_name "f3d_vtkext")
  set(headers_component "plugin_sdk")
else()
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
    # When building as a static lib or against a static VTK, the module is not installed at all
    set(f3d_vtk_no_install "NO_INSTALL")
  endif()
  set(f3d_vtk_force_static "FORCE_STATIC")
endif()

vtk_module_build(
  MODULES ${modules}
  ENABLE_WRAPPING OFF
  INSTALL_EXPORT ${export_name}
  INSTALL_HEADERS ON
  HEADERS_COMPONENT ${headers_component}
  ${f3d_vtkext_headers_exclude}
  HEADERS_DESTINATION "include/f3d"
  TARGETS_COMPONENT library
  PACKAGE "f3d_vtkext")

foreach (module IN LISTS modules)
  vtk_module_compile_options(${module} PUBLIC ${f3d_compile_options_public})
  vtk_module_compile_options(${module} PRIVATE ${f3d_compile_options_private})
  vtk_module_link_options(${module} PUBLIC ${f3d_link_options_public})
  vtk_module_set_properties(${module} CXX_STANDARD 17)
  if(F3D_STRICT_BUILD AND MSVC)
    # There are warnings in VTK related to deprecated features in C++17
    vtk_module_definitions(${module} PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
  endif()
endforeach ()
