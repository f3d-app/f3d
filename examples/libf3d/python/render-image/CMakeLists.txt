cmake_minimum_required(VERSION 3.5)

project(python-render-image)

find_package(f3d REQUIRED)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_python_render-image COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/render_image.py" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" "${CMAKE_CURRENT_BINARY_DIR}/cow.png")
  add_test(NAME test_python_check-image COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/check_image.py" "${CMAKE_CURRENT_BINARY_DIR}/cow.png" "${CMAKE_CURRENT_SOURCE_DIR}/baselines/cow.png")
  set_tests_properties(test_python_check-image PROPERTIES DEPENDS "test_render-image")
endif()
