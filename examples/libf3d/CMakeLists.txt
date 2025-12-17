cmake_minimum_required(VERSION 3.10)

project(libf3d-examples)

option(BUILD_TESTING "Build the example tests" OFF)
if(BUILD_TESTING)
  enable_testing()
endif()

option(F3D_EXAMPLES_EXTERNAL_QT "Enable external-qt example (requires qt)" OFF)

option(F3D_ENABLE_CPP_EXAMPLES "Enable the C++ examples" ON)
if (F3D_ENABLE_CPP_EXAMPLES)
  add_subdirectory(cpp)
endif ()

option(F3D_ENABLE_C_EXAMPLES "Enable the C examples" OFF)
if (F3D_ENABLE_C_EXAMPLES)
  add_subdirectory(c)
endif ()

option(F3D_ENABLE_PYTHON_EXAMPLES "Enable the Python examples" OFF)
if (F3D_ENABLE_PYTHON_EXAMPLES)
  add_subdirectory(python)
endif ()

option(F3D_ENABLE_JAVA_EXAMPLES "Enable the Java examples" OFF)
if (F3D_ENABLE_JAVA_EXAMPLES)
  add_subdirectory(java)
endif ()
