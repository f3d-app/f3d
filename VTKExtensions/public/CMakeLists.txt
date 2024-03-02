vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

# Although we have a single module
# we need to follow the full VTK module logic here
vtk_module_scan(
  MODULE_FILES      ${vtk_module_files}
  PROVIDES_MODULES  modules
  WANT_BY_DEFAULT   ON
  ENABLE_TESTS      ${BUILD_TESTING})

# We need HEADERS_COMPONENT because of a bug in vtk_module_build in VTK <= 9.1
# See https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9192
# Installed as part of the plugin SDK
vtk_module_build(
  MODULES ${modules}
  INSTALL_EXPORT "f3dVTKExtensionsTargets"
  INSTALL_HEADERS ON
  HEADERS_COMPONENT plugin_sdk
  HEADERS_EXCLUDE_FROM_ALL ON
  HEADERS_DESTINATION "include/f3dPlugin"
  TARGETS_COMPONENT vtkext
  PACKAGE "f3dPlugin")
