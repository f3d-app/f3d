list(APPEND pyf3dTests_list
     test_animation.py
     test_camera.py
     test_interactor.py
     test_image.py
     test_log.py
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
     test_interactor_start.py
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  list(APPEND pyf3dTests_list
     test_scene_buffer.py
    )
endif()

list(APPEND pyf3dTestsNoRender_list
     test_options.py
     test_engine.py
     test_utils.py
    )

# Add all the ADD_TEST for each test
foreach(test ${pyf3dTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test(NAME libf3d_bindings_py::${TName} COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_LIST_DIR}/${test})
  set_tests_properties(libf3d_bindings_py::${TName} PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}$<${F3D_MULTI_CONFIG_GENERATOR}:/$<CONFIG>>"
    FAIL_REGULAR_EXPRESSION "[^X]FAILURES")
  set_tests_properties(libf3d_bindings_py::${TName} PROPERTIES LABELS "libf3d;bindings;python")
  if (NOT F3D_TESTING_ENABLE_RENDERING_TESTS AND NOT ${test} IN_LIST pyf3dTestsNoRender_list)
    set_tests_properties(libf3d_bindings_py::${TName} PROPERTIES DISABLED ON)
  endif ()
endforeach()
