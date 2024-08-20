cmake_minimum_required(VERSION 3.5)

project(use-options-variant)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(use-options-variant main.cxx)
target_link_libraries(use-options-variant f3d::libf3d)
set_target_properties(use-options-variant PROPERTIES CXX_STANDARD 17)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_use-options-variant COMMAND "$<TARGET_FILE:use-options-variant>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
  set_tests_properties(test_use-options-variant PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of points: 2903")
endif()
