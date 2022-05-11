#TODO add more tests
list(APPEND libf3dSDKTests_list
     TestSDKRenderAndInteract.cxx
     TestSDKEngineExceptions.cxx
     TestSDKOptions.cxx
    )

# Test image comparison only with VTK > 9.0.1
if(VTK_VERSION VERSION_GREATER 9.0.1)
  list(APPEND libf3dSDKTests_list
    TestSDKCompareWithFile.cxx
    TestSDKInteractionDirectory.cxx
    )
endif()

# Animation works only with a recent version of VTK
if(VTK_VERSION VERSION_GREATER 9.0.20201016)
  list(APPEND libf3dSDKTests_list
    TestSDKAnimation.cxx
    )
endif()

# create the testing file and list of tests
create_test_sourcelist(_libf3dSDKTests libf3dSDKTests.cxx ${libf3dSDKTests_list})

# add the executable
add_executable(libf3dSDKTests ${_libf3dSDKTests})
set_target_properties(libf3dSDKTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Add all the ADD_TEST for each test
foreach (test ${libf3dSDKTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME libf3d::${TName} COMMAND libf3dSDKTests ${TName}  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
endforeach ()

target_link_libraries(libf3dSDKTests libf3d)
target_compile_features(libf3dSDKTests PRIVATE cxx_std_11)
