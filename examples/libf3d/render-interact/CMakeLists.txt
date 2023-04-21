cmake_minimum_required(VERSION 3.1)

project(render-interact)

find_package(f3d REQUIRED)

add_executable(render-interact main.cxx)
target_link_libraries(render-interact f3d::libf3d)
set_target_properties(render-interact PROPERTIES CXX_STANDARD 11)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_render-interact COMMAND "$<TARGET_FILE:render-interact>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
  set_tests_properties(test_render-interact PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of points: 2903")
endif()
