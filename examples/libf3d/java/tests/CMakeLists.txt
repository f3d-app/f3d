cmake_minimum_required(VERSION 3.10)

get_filename_component(F3D_JAVA_LIBRARY_DIR "${f3d_DIR}/../.." ABSOLUTE)

set(java_test_args "--enable-native-access=ALL-UNNAMED")
if (WIN32)
  set(F3D_JAVA_LIBRARY_DIR "${F3D_JAVA_LIBRARY_DIR}/../bin")
endif()
if(APPLE)
  list(APPEND java_test_args "-XstartOnFirstThread")
endif()

# interactive-app
add_test(NAME test_interactive-app-java COMMAND ${Java_JAVA_EXECUTABLE}
  -Djava.library.path=${F3D_JAVA_LIBRARY_DIR}
  ${java_test_args}
  -jar $<SHELL_PATH:$<TARGET_PROPERTY:interactive-app-java,JAR_FILE>>
  "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
set_tests_properties(test_interactive-app-java PROPERTIES TIMEOUT 60)
