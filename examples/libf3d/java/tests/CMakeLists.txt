cmake_minimum_required(VERSION 3.10)

get_filename_component(F3D_JAVA_LIBRARY_DIR "${f3d_DIR}/../.." ABSOLUTE)
get_filename_component(F3D_JAR "${f3d_DIR}/../../../share/java/f3d.jar" ABSOLUTE)

set(java_test_args "")
if (WIN32)
  list(APPEND java_test_args "--enable-native-access=ALL-UNNAMED")
endif()
if(APPLE)
  list(APPEND java_test_args "-XstartOnFirstThread")
endif()

# interactive-app
add_test(NAME test_interactive-app-java COMMAND ${Java_JAVA_EXECUTABLE}
  ${java_test_args}
  -jar $<SHELL_PATH:$<TARGET_PROPERTY:interactive-app-java,JAR_FILE>>
  "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
if(WIN32)
  set_tests_properties(test_interactive-app-java PROPERTIES ENVIRONMENT "PATH=${F3D_JAVA_LIBRARY_DIR}/../bin")
elseif(APPLE)
  set_tests_properties(test_interactive-app-java PROPERTIES ENVIRONMENT "DYLD_LIBRARY_PATH=${F3D_JAVA_LIBRARY_DIR}")
else()
  set_tests_properties(test_interactive-app-java PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${F3D_JAVA_LIBRARY_DIR}")
endif()
