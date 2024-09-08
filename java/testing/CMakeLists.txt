list(APPEND javaf3dTests_list
     ${CMAKE_CURRENT_SOURCE_DIR}/TestJavaBindings.java
    )

set(java_test_args "-ea") # enable assertions
if(APPLE)
  # On macOS, this argument is mandatory in order to create native window
  list(APPEND java_test_args "-XstartOnFirstThread")
endif()

set(java_test_disabled OFF)

if(NOT F3D_TESTING_ENABLE_RENDERING_TESTS)
  set(java_test_disabled ON)
endif ()

# Add all the ADD_TEST for each test
foreach(test_file ${javaf3dTests_list})
  get_filename_component (TName ${test_file} NAME_WE)

  add_test(NAME javaf3d::${TName}
    WORKING_DIRECTORY "$<SHELL_PATH:$<TARGET_FILE_DIR:javaf3d>>" # path to the JNI library
    COMMAND ${Java_JAVA_EXECUTABLE}
      -Djava.library.path=$<SHELL_PATH:$<TARGET_FILE_DIR:javaf3d>> # path to the JNI library
      ${java_test_args}
      -cp $<SHELL_PATH:$<TARGET_PROPERTY:f3d-jar,JAR_FILE>> # path to the JAR file
      $<SHELL_PATH:${test_file}> ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/
    )

    set_tests_properties(javaf3d::${TName} PROPERTIES DISABLED ${java_test_disabled})
endforeach()
