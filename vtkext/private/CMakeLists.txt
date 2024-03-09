vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

vtk_module_scan(
  MODULE_FILES      ${vtk_module_files}
  PROVIDES_MODULES  modules
  WANT_BY_DEFAULT   ON
  ENABLE_TESTS      ${BUILD_TESTING})

# if libf3d is built as a static library, VTK extension static libraries
# must be exported as well for proper linkage
set(export_name "")
set(f3d_vtk_no_install "")
if(NOT BUILD_SHARED_LIBS)
  set(export_name "f3dTargets")
else()
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
    set(f3d_vtk_no_install "NO_INSTALL")
  endif()
endif()

# We need HEADERS_COMPONENT because of a bug in vtk_module_build in VTK <= 9.1
# See https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9192
vtk_module_build(
  MODULES ${modules}
  INSTALL_EXPORT ${export_name}
  INSTALL_HEADERS OFF
  HEADERS_COMPONENT vtkext
  TARGETS_COMPONENT vtkext
  PACKAGE "f3d-VTKExtensionsPrivate")

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
