list(APPEND pyf3dTests_list
     test_camera.py
     test_image.py
     test_loader.py
     test_options.py
     test_utils.py
    )

if(NOT F3D_MACOS_BUNDLE)
  list(APPEND pyf3dTests_list
       test_plugins.py
      )
endif()

# Test image comparison only with VTK > 9.0.1
if(VTK_VERSION VERSION_GREATER 9.0.1)

  # A deprecated test use image comparison
  if(NOT F3D_EXCLUDE_DEPRECATED)
      list(APPEND pyf3dTests_list
          test_deprecated.py)
  endif()

  list(APPEND pyf3dTests_list
       test_image_compare.py
      )
endif()

list(APPEND pyf3dTestsNoRender_list
     test_options.py
     test_plugins.py
     test_utils.py
    )

# Add all the ADD_TEST for each test
foreach(test ${pyf3dTests_list})
  get_filename_component (TName TestPython_${test} NAME_WE)
  string(REPLACE "test_" "" TName ${TName})
  add_test(NAME pyf3d::${TName} COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_LIST_DIR}/${test})
  set_tests_properties(pyf3d::${TName} PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}$<${F3D_MULTI_CONFIG_GENERATOR}:/$<CONFIG>>"
    FAIL_REGULAR_EXPRESSION "FAILURES")
  if (NOT F3D_TESTING_ENABLE_RENDERING_TESTS AND NOT ${test} IN_LIST pyf3dTestsNoRender_list)
    set_tests_properties(pyf3d::${TName} PROPERTIES DISABLED ON)
  endif ()
endforeach()
