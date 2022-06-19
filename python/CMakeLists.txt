find_package(pybind11 2.2 REQUIRED)

pybind11_add_module(pyf3d F3DPythonBindings.cxx)

target_compile_features(pyf3d PRIVATE cxx_std_14)

target_link_libraries(pyf3d PRIVATE libf3d)

set_target_properties(pyf3d PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  CXX_VISIBILITY_PRESET hidden
  OUTPUT_NAME "f3d"
  )

if (DEFINED SKBUILD)
  # If we are building a python wheel, we import _f3d.* in __init__.py
  target_compile_definitions(pyf3d PRIVATE F3D_PYTHONLIB_NAME=_f3d)
endif()

if(WIN32)
  # On Windows, the python module needs to be in the same folder than f3d.dll
  # Usage of PATH to find the DLL is not possible, see https://stackoverflow.com/a/64303856/2609654
  set_target_properties(pyf3d PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

if (APPLE OR UNIX)
  set_target_properties(pyf3d PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif ()

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()
