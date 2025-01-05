list(APPEND pyf3dTests_list
     test_camera.py
     test_interactor.py
     test_image.py
     test_options.py
     test_utils.py
    )

if(NOT F3D_MACOS_BUNDLE)
  list(APPEND pyf3dTests_list
       test_engine.py
      )
endif()

list(APPEND pyf3dTests_list
     test_image_compare.py
     test_scene.py
    )

list(APPEND pyf3dTestsNoRender_list
     test_options.py
     test_engine.py
     test_utils.py
    )

# These tests needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10155
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20230426)
list(APPEND pyf3dTests_list
     test_interactor_start.py)
endif ()

# Add all the ADD_TEST for each test
foreach(test ${pyf3dTests_list})
  get_filename_component (TName TestPython_${test} NAME_WE)
  string(REPLACE "test_" "" TName ${TName})
  add_test(NAME pyf3d::${TName} COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_LIST_DIR}/${test})
  set_tests_properties(pyf3d::${TName} PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}$<${F3D_MULTI_CONFIG_GENERATOR}:/$<CONFIG>>"
    FAIL_REGULAR_EXPRESSION "[^X]FAILURES")
  if (NOT F3D_TESTING_ENABLE_RENDERING_TESTS AND NOT ${test} IN_LIST pyf3dTestsNoRender_list)
    set_tests_properties(pyf3d::${TName} PROPERTIES DISABLED ON)
  endif ()
endforeach()
