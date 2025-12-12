cmake_minimum_required(VERSION 3.10)

# interactive-app
add_test(NAME test_interactive-app-c COMMAND "$<TARGET_FILE:interactive-app-c>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
