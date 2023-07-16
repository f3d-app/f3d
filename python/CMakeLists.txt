cmake_minimum_required(VERSION 3.21)

project(pyf3d)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(f3dPython)

find_package(Python 3.6 COMPONENTS Interpreter Development)
find_package(pybind11 2.2 REQUIRED)

pybind11_add_module(pyf3d MODULE F3DPythonBindings.cxx)

# In case pyf3d is built separately, we need to retrieve the existing libf3d
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED)
  target_link_libraries(pyf3d PRIVATE f3d::libf3d)
  target_include_directories(pyf3d PRIVATE "${f3d_INCLUDE_DIR}/f3d")
else()
  target_link_libraries(pyf3d PRIVATE libf3d)
endif()

set(f3d_python_package_name "f3d")
set(f3d_python_package_suffix "python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
set(f3d_python_lib_output "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${f3d_python_package_suffix}/${f3d_python_package_name}")

set_target_properties(pyf3d PROPERTIES
  CXX_STANDARD 14
  CXX_VISIBILITY_PRESET hidden
  LIBRARY_OUTPUT_DIRECTORY "${f3d_python_lib_output}"
  )

if (APPLE)
  set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "@loader_path/../../../${CMAKE_INSTALL_LIBDIR}")
elseif (UNIX)
  set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "$ORIGIN/../../../${CMAKE_INSTALL_LIBDIR}")
endif()
set(f3d_python_install_path "${CMAKE_INSTALL_LIBDIR}/${f3d_python_package_suffix}/${f3d_python_package_name}")

# generate Windows fixup for build
set(F3D_ABSOLUTE_DLLS_FIXUP "")
set(F3D_RELATIVE_DLLS_FIXUP "")
if(WIN32)
  f3d_python_windows_dll_fixup(PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "$<TARGET_FILE_DIR:VTK::CommonCore>" OUTPUT F3D_ABSOLUTE_DLLS_FIXUP)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${CMAKE_CURRENT_BINARY_DIR}/__init__.py-build" @ONLY)

file(GENERATE
  OUTPUT "${f3d_python_lib_output}/__init__.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/__init__.py-build"
)

# generate Windows fixup for install
set(F3D_ABSOLUTE_DLLS_FIXUP "")
set(F3D_RELATIVE_DLLS_FIXUP "")
if(WIN32)
  set(f3d_python_dll_install_bin_path "../../../${CMAKE_INSTALL_BINDIR}")
  f3d_python_windows_dll_fixup(PATHS "${f3d_python_dll_install_bin_path}" OUTPUT F3D_RELATIVE_DLLS_FIXUP)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${CMAKE_CURRENT_BINARY_DIR}/__init__.py-install" @ONLY)

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# installing
install(TARGETS pyf3d
  LIBRARY DESTINATION ${f3d_python_install_path} COMPONENT pythonmodule)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py-install" RENAME "__init__.py"
  DESTINATION ${f3d_python_install_path} COMPONENT pythonmodule)
