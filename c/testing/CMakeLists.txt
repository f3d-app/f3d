list(APPEND f3d_c_api_tests_list
     test_image_c_api.c
    )

create_test_sourcelist(_f3d_c_api_driver_src f3d_c_api_tests.c ${f3d_c_api_tests_list})

# add the executable
add_executable(f3d_c_api_tests ${_f3d_c_api_driver_src})
set_target_properties(f3d_c_api_tests PROPERTIES
  C_VISIBILITY_PRESET hidden
  )

target_link_libraries(f3d_c_api_tests PRIVATE f3d_c_api)

# Add all the ADD_TEST for each test
foreach (test ${f3d_c_api_tests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME f3d_c_api::${TName} COMMAND f3d_c_api_tests ${TName} "${F3D_SOURCE_DIR}/testing/" "${CMAKE_BINARY_DIR}/Testing/Temporary/" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set_tests_properties(f3d_c_api::${TName} PROPERTIES TIMEOUT 30)
endforeach ()
