vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

vtk_module_scan(
  MODULE_FILES      ${vtk_module_files}
  PROVIDES_MODULES  modules
  WANT_BY_DEFAULT   ON
  ENABLE_TESTS      ${BUILD_TESTING})

vtk_module_build(MODULES ${modules})

foreach (module IN LISTS modules)
  if(NOT "${libf3d_compile_options_public}" STREQUAL "")
    vtk_module_compile_options(${module} PUBLIC ${libf3d_compile_options_public})
  endif()
  if(NOT "${libf3d_compile_options_private}" STREQUAL "")
    vtk_module_compile_options(${module} PRIVATE ${libf3d_compile_options_private})
  endif()
  if (NOT "${libf3d_link_options_public}" STREQUAL "")
    vtk_module_link_options(${module} PUBLIC ${libf3d_link_options_public})
  endif()
endforeach ()

