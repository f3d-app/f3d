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
  ${CMAKE_CURRENT_SOURCE_DIR}/interactive-app/interactive_app.py ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/cow.vtp
)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/render-interact/render_interact.py
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
