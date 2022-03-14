vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

vtk_module_scan(
  MODULE_FILES      ${vtk_module_files}
  PROVIDES_MODULES  modules
  WANT_BY_DEFAULT   ON
  ENABLE_TESTS      ON)

vtk_module_build(MODULES ${modules})
