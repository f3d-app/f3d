cmake_minimum_required(VERSION 3.10)

project(multi-files)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(multi-files main.cxx)
target_link_libraries(multi-files f3d::libf3d)
set_target_properties(multi-files PROPERTIES CXX_STANDARD 17)

if(UNIX AND NOT APPLE)
  target_link_libraries(multi-files stdc++fs)
endif()

# Simple testing
if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_multi-files COMMAND "$<TARGET_FILE:multi-files>" "${CMAKE_CURRENT_SOURCE_DIR}/data/")
  set_tests_properties(test_multi-files PROPERTIES
    PASS_REGULAR_EXPRESSION "Scene bounding box: -0.487464,1,-0.487464,1,-0.5,1")
endif()
