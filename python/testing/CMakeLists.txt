list(APPEND pyf3dTests_list
     TestPythonOptions.py
     TestPythonCamera.py
     TestPythonImageData.py
    )

# Test image comparison only with VTK > 9.0.1
if(VTK_VERSION VERSION_GREATER 9.0.1)
  list(APPEND pyf3dTests_list
       TestPythonCompareWithFile.py
      )
endif()

list(APPEND pyf3dTestsNoRender_list
     TestPythonOptions
    )

# Add all the ADD_TEST for each test
foreach(test ${pyf3dTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test(NAME pyf3d::${TName} COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/${test} $<TARGET_FILE_DIR:VTK::CommonCore> ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
  set_tests_properties(pyf3d::${TName} PROPERTIES
    ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:pyf3d>"
    FAIL_REGULAR_EXPRESSION "AssertionError")
  if (NOT F3D_TESTING_ENABLE_RENDERING_TESTS AND NOT ${TName} IN_LIST pyf3dTestsNoRender_list)
    set_tests_properties(pyf3d::${TName} PROPERTIES DISABLED ON)
  endif ()
endforeach()
