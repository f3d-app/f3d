cmake_minimum_required(VERSION 3.10)

project(libf3d-examples)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(cpp)
if (F3D_BINDINGS_C)
  add_subdirectory(c)
endif ()
if (F3D_BINDINGS_PYTHON)
  add_subdirectory(python)
endif ()
if (F3D_BINDINGS_JAVA)
  add_subdirectory(java)
endif ()
