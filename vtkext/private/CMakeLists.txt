vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

vtk_module_scan(
  MODULE_FILES      ${vtk_module_files}
  PROVIDES_MODULES  modules
  WANT_BY_DEFAULT   ON
  ENABLE_TESTS      ${BUILD_TESTING})

vtk_module_build(
  MODULES ${modules}
  ENABLE_WRAPPING OFF
  INSTALL_HEADERS OFF
  PACKAGE "f3d_vtkext_private")

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
