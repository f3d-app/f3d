cmake_minimum_required(VERSION 3.10)

get_filename_component(F3D_JAVA_LIBRARY_DIR "${f3d_DIR}/../.." ABSOLUTE)
get_filename_component(F3D_JAR "${f3d_DIR}/../../../share/java/f3d.jar" ABSOLUTE)

set(java_test_args "")
if(APPLE)
  list(APPEND java_test_args "-XstartOnFirstThread")
endif()

if(WIN32)
  set(CP_SEP "$<SEMICOLON>")
else()
  set(CP_SEP ":")
endif()

# interactive-app
add_test(NAME test_interactive-app-java COMMAND ${Java_JAVA_EXECUTABLE}
  ${java_test_args}
  -Djava.library.path=${F3D_JAVA_LIBRARY_DIR}
  -cp "$<TARGET_PROPERTY:interactive-app-java,JAR_FILE>${CP_SEP}${F3D_JAR}"
  InteractiveApp
  "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
if(WIN32)
  set_tests_properties(test_interactive-app-java PROPERTIES ENVIRONMENT "PATH=${F3D_JAVA_LIBRARY_DIR}/../bin")
elseif(APPLE)
  set_tests_properties(test_interactive-app-java PROPERTIES ENVIRONMENT "DYLD_LIBRARY_PATH=${F3D_JAVA_LIBRARY_DIR}")
else()
  set_tests_properties(test_interactive-app-java PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${F3D_JAVA_LIBRARY_DIR}")
endif()
