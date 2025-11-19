find_package(Python3 COMPONENTS Interpreter REQUIRED)

# check-engine
add_test(
  NAME test_check-engine
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/check-engine/check_engine.py
)

# img-cmp
add_test(
  NAME test_img-cmp
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/img-cmp/img-cmp.py ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/cow.png ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/cow.png
)

# interactive-app
add_test(
  NAME test_interactive-app
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/interactive-app/interactive_app.py ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/cow.vtp --timeout 1
)

# offscreen-thumbnail
add_test(
  NAME test_offscreen-thumbnail
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/offscreen-thumbnail/offscreen_thumbnail.py ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/cow.vtp ${CMAKE_CURRENT_BINARY_DIR}/thumbnail.png 256
)
add_test(
  NAME test_check-offscreen-thumbnail
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/img-cmp/img-cmp.py ${CMAKE_CURRENT_BINARY_DIR}/thumbnail.png ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/thumbnail.png
)
set_tests_properties(test_check-offscreen-thumbnail PROPERTIES DEPENDS test_offscreen-thumbnail)

# render-image
add_test(
  NAME test_render-image
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/render-image/render_image.py
)

# render-interact
add_test(
  NAME test_render-interact
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/render-interact/render_interact.py --timeout 1
)

# render-terminal
add_test(
  NAME test_render-terminal
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/render-terminal/render_terminal.py
)

# tkinter
add_test(
  NAME test_tkinter
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/tkinter/minimal_tkinter.py
)
