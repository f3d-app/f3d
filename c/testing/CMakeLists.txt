list(APPEND f3d_c_api_tests_list
  test_camera_c_api.c
  test_engine_c_api.c
  test_image_c_api.c
  test_interactor_c_api.c
  test_log_c_api.c
  test_options_c_api.c
  test_scene_c_api.c
  test_types_c_api.c
  test_utils_c_api.c
  test_window_c_api.c
)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  list(APPEND f3d_c_api_tests_list
    test_scene_buffer_c_api.c
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
  add_test(NAME c_api::${TName} COMMAND f3d_c_api_tests ${TName} "${F3D_SOURCE_DIR}/testing/" "${CMAKE_BINARY_DIR}/Testing/Temporary/" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set_tests_properties(c_api::${TName} PROPERTIES TIMEOUT 30)
endforeach ()
