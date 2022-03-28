# create the testing file and list of tests
create_test_sourcelist (Tests
  sdk_tests.cxx
  simple_sdk_test.cxx
  render_and_interact_test.cxx
  )

# add the executable
add_executable (sdk_tests ${Tests})

# remove the test driver source file
set (TestsToRun ${Tests})
remove (TestsToRun sdk_tests.cxx)

# Add all the ADD_TEST for each test
foreach (test ${TestsToRun})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME libf3d::${TName} COMMAND sdk_tests ${TName}  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
endforeach ()

target_link_libraries(sdk_tests libf3d)
