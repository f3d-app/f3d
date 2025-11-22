cmake_minimum_required(VERSION 3.10)

# check-engine
add_test(NAME test_check-engine COMMAND "$<TARGET_FILE:check-engine>")

# multi-files
add_test(NAME test_multi-files COMMAND "$<TARGET_FILE:multi-files>" "${CMAKE_CURRENT_SOURCE_DIR}/data/mb")
set_tests_properties(test_multi-files PROPERTIES
        PASS_REGULAR_EXPRESSION "Scene bounding box: -0.487464 ≤ x ≤ 1, -0.487464 ≤ y ≤ 1, -0.5 ≤ z ≤ 1")

# render-image
add_test(NAME test_render-image COMMAND "$<TARGET_FILE:render-image>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" "${CMAKE_CURRENT_BINARY_DIR}/cow.png")
add_test(NAME test_check-image COMMAND "$<TARGET_FILE:check-image>" "${CMAKE_CURRENT_BINARY_DIR}/cow.png" "${CMAKE_CURRENT_SOURCE_DIR}/baselines/cow.png")
set_tests_properties(test_check-image PROPERTIES DEPENDS "test_render-image")

# render-interact
add_test(NAME test_render-interact COMMAND "$<TARGET_FILE:render-interact>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
set_tests_properties(test_render-interact PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# use-options-string
add_test(NAME test_use-options-string COMMAND "$<TARGET_FILE:use-options-string>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
set_tests_properties(test_use-options-string PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# use-options-struct
add_test(NAME test_use-options-struct COMMAND "$<TARGET_FILE:use-options-struct>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
set_tests_properties(test_use-options-struct PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# use-options-variant
add_test(NAME test_use-options-variant COMMAND "$<TARGET_FILE:use-options-variant>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp")
set_tests_properties(test_use-options-variant PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")
