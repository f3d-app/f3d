#TODO add more tests

# create the testing file and list of tests
create_test_sourcelist(libf3dSDKTests_list
  libf3dSDKTests.cxx
  TestSDKCompareWithFile.cxx
  TestSDKRenderAndInteract.cxx
  )

# add the executable
add_executable(libf3dSDKTests ${libf3dSDKTests_list})
set_target_properties(libf3dSDKTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# remove the test driver source file
remove (libf3dSDKTests_list libf3dSDKTests.cxx)

# Add all the ADD_TEST for each test
foreach (test ${libf3dSDKTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME libf3d::${TName} COMMAND libf3dSDKTests ${TName}  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
endforeach ()

target_link_libraries(libf3dSDKTests libf3d)
target_compile_features(libf3dSDKTests PRIVATE cxx_std_11)
