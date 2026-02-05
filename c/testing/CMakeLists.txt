list(APPEND f3d_c_api_tests_list
  test_camera.c
  test_engine.c
  test_image.c
  test_interactor.c
  test_log.c
  test_options.c
  test_scene.c
  test_types.c
  test_utils.c
  test_window.c
)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  list(APPEND f3d_c_api_tests_list
    test_scene_buffer.c
    )
endif()

set(CMAKE_TESTDRIVER_EXTRA_INCLUDES "")
set(CMAKE_TESTDRIVER_ARGVC_FUNCTION "")
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "")
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "")
create_test_sourcelist(_f3d_c_api_driver_src f3d_c_api_tests.c ${f3d_c_api_tests_list})

# add the executable
add_executable(f3d_c_api_tests ${_f3d_c_api_driver_src})

target_link_libraries(f3d_c_api_tests PRIVATE c_api)
target_compile_definitions(f3d_c_api_tests PRIVATE F3D_TESTING_DATA_DIR="${F3D_SOURCE_DIR}/testing/data/")

# Add all the ADD_TEST for each test
foreach (test ${f3d_c_api_tests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test(NAME libf3d_bindings_c::${TName} COMMAND f3d_c_api_tests ${TName} "${F3D_SOURCE_DIR}/testing/" "${CMAKE_BINARY_DIR}/Testing/Temporary/" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set_tests_properties(libf3d_bindings_c::${TName} PROPERTIES TIMEOUT 30)
  set_tests_properties(libf3d_bindings_c::${TName} PROPERTIES LABELS "libf3d;bindings;c")
endforeach ()
