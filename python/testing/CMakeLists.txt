list(APPEND pyf3dTests_list
     TestPythonCamera.py
     TestPythonImageData.py
     TestPythonOptions.py
     TestPythonUtils.py
    )

if(NOT F3D_EXCLUDE_DEPRECATED)
  list(APPEND pyf3dTests_list
       TestPythonImageDataDeprecated.py)
endif()

if(NOT F3D_MACOS_BUNDLE)
  list(APPEND pyf3dTests_list
       TestPythonPlugins.py
      )
endif()

# Test image comparison only with VTK > 9.0.1
if(VTK_VERSION VERSION_GREATER 9.0.1)
  list(APPEND pyf3dTests_list
       TestPythonCompareWithFile.py
      )
endif()

list(APPEND pyf3dTestsNoRender_list
     TestPythonOptions
     TestPythonPlugins
     TestPythonUtils
    )

# Add all the ADD_TEST for each test
foreach(test ${pyf3dTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test(NAME pyf3d::${TName} COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/${test} ${F3D_SOURCE_DIR} ${CMAKE_BINARY_DIR})
  set_tests_properties(pyf3d::${TName} PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${f3d_python_package_suffix}"
    FAIL_REGULAR_EXPRESSION "AssertionError")
  if (NOT F3D_TESTING_ENABLE_RENDERING_TESTS AND NOT ${TName} IN_LIST pyf3dTestsNoRender_list)
    set_tests_properties(pyf3d::${TName} PROPERTIES DISABLED ON)
  endif ()
endforeach()
