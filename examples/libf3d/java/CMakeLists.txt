cmake_minimum_required(VERSION 3.10)

add_subdirectory(interactive-app)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()
