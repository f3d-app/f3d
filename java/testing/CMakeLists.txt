list(APPEND javaf3dTests_list
     ${CMAKE_CURRENT_SOURCE_DIR}/TestJavaBindings.java
    )

set(java_test_args "-ea") # enable assertions
if(APPLE)
  # On macOS, this argument is mandatory in order to create native window
  list(APPEND java_test_args "-XstartOnFirstThread")
endif()

# On macOS, VTK 9.1 is crashing in modifyDPIForBackingScaleFactorOfWindow, see:
# https://gitlab.kitware.com/vtk/vtk/-/merge_requests/6845 (introduces the crash)
# https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9372 (fixes the crash)
# Disabling the tests if we are between these two commits
set(java_test_disabled OFF)
if(APPLE AND VTK_VERSION VERSION_GREATER 9.0.0 AND VTK_VERSION VERSION_LESS_EQUAL 9.1.20220617)
  set(java_test_disabled ON)
endif()

# Add all the ADD_TEST for each test
foreach(test_file ${javaf3dTests_list})
  get_filename_component (TName ${test_file} NAME_WE)

  add_test(NAME javaf3d::${TName}
    WORKING_DIRECTORY "$<SHELL_PATH:$<TARGET_FILE_DIR:javaf3d>>" # path to the JNI library
    COMMAND ${Java_JAVA_EXECUTABLE}
      -Djava.library.path=$<SHELL_PATH:$<TARGET_FILE_DIR:javaf3d>> # path to the JNI library
      ${java_test_args}
      -cp $<SHELL_PATH:$<TARGET_PROPERTY:f3d-jar,JAR_FILE>> # path to the JAR file
      $<SHELL_PATH:${test_file}> ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/
    )

    set_tests_properties(javaf3d::${TName} PROPERTIES DISABLED ${java_test_disabled})
endforeach()
