cmake_minimum_required(VERSION 3.10)

# for image comparisons
add_executable(check-image check.cxx)
target_link_libraries(check-image f3d::libf3d)
set_target_properties(check-image PROPERTIES CXX_STANDARD 17)

# check-engine
add_test(NAME test_check-engine COMMAND "$<TARGET_FILE:check-engine>")

# interactive-app
add_test(NAME test_interactive-app COMMAND "$<TARGET_FILE:interactive-app>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)

# multi-files
add_test(NAME test_multi-files COMMAND "$<TARGET_FILE:multi-files>" "${CMAKE_CURRENT_SOURCE_DIR}/data/mb" 1)
set_tests_properties(test_multi-files PROPERTIES
        PASS_REGULAR_EXPRESSION "Scene bounding box: -0.487464 ≤ x ≤ 1, -0.487464 ≤ y ≤ 1, -0.5 ≤ z ≤ 1")

# offscreen-thumbnail
add_test(NAME test_offscreen-thumbnail COMMAND "$<TARGET_FILE:offscreen-thumbnail>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" "${CMAKE_CURRENT_BINARY_DIR}/thumbnail.png" 256 256)
add_test(NAME test_check-offscreen-thumbnail COMMAND "$<TARGET_FILE:check-image>" "${CMAKE_CURRENT_BINARY_DIR}/thumbnail.png" "${CMAKE_CURRENT_SOURCE_DIR}/baselines/thumbnail.png")
set_tests_properties(test_check-offscreen-thumbnail PROPERTIES DEPENDS "test_offscreen-thumbnail")

# render-image
add_test(NAME test_render-image COMMAND "$<TARGET_FILE:render-image>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" "${CMAKE_CURRENT_BINARY_DIR}/cow.png")
add_test(NAME test_check-image COMMAND "$<TARGET_FILE:check-image>" "${CMAKE_CURRENT_BINARY_DIR}/cow.png" "${CMAKE_CURRENT_SOURCE_DIR}/baselines/cow.png")
set_tests_properties(test_check-image PROPERTIES DEPENDS "test_render-image")

# render-interact
add_test(NAME test_render-interact COMMAND "$<TARGET_FILE:render-interact>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
set_tests_properties(test_render-interact PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# use-options-string
add_test(NAME test_use-options-string COMMAND "$<TARGET_FILE:use-options-string>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
set_tests_properties(test_use-options-string PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# use-options-struct
add_test(NAME test_use-options-struct COMMAND "$<TARGET_FILE:use-options-struct>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
set_tests_properties(test_use-options-struct PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# use-options-variant
add_test(NAME test_use-options-variant COMMAND "$<TARGET_FILE:use-options-variant>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
set_tests_properties(test_use-options-variant PROPERTIES
        PASS_REGULAR_EXPRESSION "Number of points: 2903")

# glfw -- only works if libf3d was built against a VTK >= 9.3.20240914
if (F3D_EXAMPLES_EXTERNAL_GLFW)
  add_test(NAME test_external-glfw COMMAND "$<TARGET_FILE:external-glfw>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
endif ()

# qt6 & qml -- only works if libf3d was built against a VTK >= 9.3.20240914
if (F3D_EXAMPLES_EXTERNAL_QT)
  add_test(NAME test_minimal_qt6 COMMAND "$<TARGET_FILE:minimal_qt6>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" "--timeout" 1)
  add_test(
    NAME test_minimal_qml
    COMMAND "$<TARGET_FILE:minimal_qml>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" "--timeout" 1
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:minimal_qml>"
  )
endif ()
