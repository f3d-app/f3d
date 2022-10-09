find_package(Java REQUIRED)
message(STATUS "Bindings: Java: Java ${Java_VERSION} found")
include(UseJava)

# With Android NDK, JNI is automatically found
if (NOT ANDROID)
  find_package(JNI REQUIRED)
  message(STATUS "Bindings: Java: JNI found")
endif()

set(F3D_JAVA_SOURCES
  Camera.java
  Engine.java
  Loader.java
  Options.java
  Window.java)

# Generate JNI headers, and builds a JAR package
set(CMAKE_JAVA_COMPILE_FLAGS -source 8 -target 8)
add_jar(f3d-jar ${F3D_JAVA_SOURCES}
  GENERATE_NATIVE_HEADERS f3d-jni-headers
  OUTPUT_NAME f3d)

add_library(javaf3d SHARED F3DJavaBindings.cxx)
target_link_libraries(javaf3d PRIVATE f3d-jni-headers)

target_include_directories(javaf3d PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(javaf3d PRIVATE libf3d ${JNI_LIBRARIES})

set_target_properties(javaf3d PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  CXX_VISIBILITY_PRESET hidden
  CXX_STANDARD 11
  OUTPUT_NAME "f3d-java"
  )

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()
