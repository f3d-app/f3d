cmake_minimum_required(VERSION 3.5)

project(python-multi-geom)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_python_multi-geom COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/multi_geom.py" "${CMAKE_CURRENT_SOURCE_DIR}/data/")
  set_tests_properties(test_multi-geom PROPERTIES
    PASS_REGULAR_EXPRESSION "Scene bounding box: -0.487464,1,-0.487464,1,-0.5,1")
endif()
