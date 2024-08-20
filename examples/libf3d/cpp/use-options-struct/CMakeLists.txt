cmake_minimum_required(VERSION 3.5)

project(use-options-struct)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(use-options-struct main.cxx)
target_compile_definitions(use-options-struct PRIVATE F3D_DISABLE_CXX17_API)
target_link_libraries(use-options-struct f3d::libf3d)
set_target_properties(use-options-struct PROPERTIES CXX_STANDARD 11)

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_use-options-struct COMMAND "$<TARGET_FILE:use-options-struct>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
  set_tests_properties(test_use-options-struct PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of points: 2903")
endif()
