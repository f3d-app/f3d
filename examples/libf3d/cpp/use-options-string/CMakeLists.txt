cmake_minimum_required(VERSION 3.5)

project(use-options-string)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(use-options-string main.cxx)
target_compile_definitions(use-options-string PRIVATE F3D_DISABLE_CXX17_API)
target_link_libraries(use-options-string f3d::libf3d)
set_target_properties(use-options-string PROPERTIES CXX_STANDARD 11)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_use-options-string COMMAND "$<TARGET_FILE:use-options-string>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
  set_tests_properties(test_use-options-string PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of points: 2903")
endif()
