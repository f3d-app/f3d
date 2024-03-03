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
# The headers and all "dev" part are installed as part of the plugin SDK
# The library itself is installed as part of the library
vtk_module_build(
  MODULES ${modules}
  INSTALL_EXPORT "f3dVTKExtensionsTargets"
  INSTALL_HEADERS ON
  HEADERS_COMPONENT plugin_sdk
#HEADERS_EXCLUDE_FROM_ALL ON
  HEADERS_DESTINATION "include/f3dPlugin"
  TARGETS_COMPONENT library
  PACKAGE "f3dPlugin")

# We need non empty cmake vars for these calls for VTK <= 9.1
# See https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9084
foreach (module IN LISTS modules)
  if(NOT "${f3d_compile_options_public}" STREQUAL "")
    vtk_module_compile_options(${module} PUBLIC ${f3d_compile_options_public})
  endif()
  if(NOT "${f3d_compile_options_private}" STREQUAL "")
    vtk_module_compile_options(${module} PRIVATE ${f3d_compile_options_private})
  endif()
  if (NOT "${f3d_link_options_public}" STREQUAL "")
    vtk_module_link_options(${module} PUBLIC ${f3d_link_options_public})
  endif()
  vtk_module_set_property(${module} PROPERTY CXX_STANDARD VALUE 17)
  if(F3D_STRICT_BUILD AND MSVC)
    # There are warnings in VTK related to deprecated features in C++17
    vtk_module_definitions(${module} PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
  endif()
endforeach ()
