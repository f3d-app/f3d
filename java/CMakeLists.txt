find_package(JNI REQUIRED)

add_library(javaf3d SHARED F3DJavaBindings.cxx)

target_include_directories(javaf3d PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(javaf3d PRIVATE libf3d ${JNI_LIBRARIES})

set_target_properties(javaf3d PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/java"
  OUTPUT_NAME "f3d-java"
  )
