list(APPEND javaf3dTests_list
  ${CMAKE_CURRENT_SOURCE_DIR}/TestCamera.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestEngine.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestImage.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestInteractor.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestLog.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestOptions.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestScene.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestTypes.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestUtils.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TestWindow.java
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  list(APPEND javaf3dTests_list
    ${CMAKE_CURRENT_SOURCE_DIR}/TestSceneBuffer.java
    )
endif()

set(java_test_args "-ea") # enable assertions
if (WIN32)
  list(APPEND java_test_args "--enable-native-access=ALL-UNNAMED")
endif()
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
  string(REPLACE "Test" "test_" TName ${TName})
  string(TOLOWER ${TName} TName)
  add_test(NAME libf3d_bindings_java::${TName}
    WORKING_DIRECTORY "$<SHELL_PATH:$<TARGET_FILE_DIR:javaf3d>>" # path to the JNI library
    COMMAND ${Java_JAVA_EXECUTABLE}
      -Djava.library.path=$<SHELL_PATH:$<TARGET_FILE_DIR:javaf3d>> # path to the JNI library
      ${java_test_args}
      -cp $<SHELL_PATH:$<TARGET_PROPERTY:f3d-jar,JAR_FILE>> # path to the JAR file
      $<SHELL_PATH:${test_file}> ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/
    )

  set_tests_properties(libf3d_bindings_java::${TName} PROPERTIES
    LABELS "libf3d;bindings;java"
    DISABLED ${java_test_disabled}
  )
endforeach()
