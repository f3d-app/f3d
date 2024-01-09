cmake_minimum_required(VERSION 3.5)

project(python-render-interact)

find_package(f3d REQUIRED)
find_package(Python 3.6 COMPONENTS Interpreter)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_python_render-interact COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/render_interact.py" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
  set_tests_properties(test_python_render-interact PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of points: 2903")
endif()
