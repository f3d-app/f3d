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

set_target_properties(pyf3d PROPERTIES
  CXX_STANDARD 14
  CXX_VISIBILITY_PRESET hidden
  LIBRARY_OUTPUT_NAME "f3d"
  )

if(WIN32)
  # On Windows, the python module needs to be in the same folder than f3d.dll
  # Usage of PATH to find the DLL is not possible, see https://stackoverflow.com/a/64303856/2609654
  set_target_properties(pyf3d PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()

if (APPLE OR UNIX)
  set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif ()

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# installing
if(WIN32)
  set(PYTHON_INSTALL_PATH ${CMAKE_INSTALL_BINDIR})
else()
  set(PYTHON_INSTALL_PATH "${CMAKE_INSTALL_LIBDIR}/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
endif()
install(TARGETS pyf3d
  LIBRARY DESTINATION ${PYTHON_INSTALL_PATH} COMPONENT pythonmodule)
