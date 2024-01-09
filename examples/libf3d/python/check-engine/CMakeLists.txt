cmake_minimum_required(VERSION 3.5)

project(python-check-engine)

find_package(f3d REQUIRED)
find_package(Python 3.6 COMPONENTS Interpreter)

if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_python_check-engine COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/check_engine.py")
endif()
