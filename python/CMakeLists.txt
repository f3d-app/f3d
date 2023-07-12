cmake_minimum_required(VERSION 3.21)

project(pyf3d)

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${f3d_python_lib_output}/__init__.py" @ONLY)

if(SKBUILD)
  if (APPLE)
    set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "@loader_path/${CMAKE_INSTALL_LIBDIR}")
  elseif (UNIX)
    set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "$ORIGIN/${CMAKE_INSTALL_LIBDIR}")
  endif()
  set(f3d_python_install_path ".")
else()
  if (APPLE)
    set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "@loader_path/../../../${CMAKE_INSTALL_LIBDIR}")
  elseif (UNIX)
    set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "$ORIGIN/../../../${CMAKE_INSTALL_LIBDIR}")
  endif()
  set(f3d_python_install_path "${CMAKE_INSTALL_LIBDIR}/${f3d_python_package_suffix}/${f3d_python_package_name}")
endif()

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# installing
install(TARGETS pyf3d
  LIBRARY DESTINATION ${f3d_python_install_path} COMPONENT pythonmodule)
install(FILES "${f3d_python_lib_output}/__init__.py"
  DESTINATION ${f3d_python_install_path} COMPONENT pythonmodule)
