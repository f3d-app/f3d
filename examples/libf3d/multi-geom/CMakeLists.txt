cmake_minimum_required(VERSION 3.1)

project(multi-geom)

find_package(f3d REQUIRED)

add_executable(multi-geom main.cxx)
target_link_libraries(multi-geom f3d::libf3d stdc++fs)
set_target_properties(multi-geom PROPERTIES CXX_STANDARD 17)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_multi-geom COMMAND "$<TARGET_FILE:multi-geom>" "${CMAKE_CURRENT_SOURCE_DIR}/data/")
  set_tests_properties(test_multi-geom PROPERTIES
    PASS_REGULAR_EXPRESSION "Scene bounding box: -0.487464,1,-0.487464,1,-0.5,1")
endif()
