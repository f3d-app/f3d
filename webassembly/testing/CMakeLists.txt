find_program(CHROMIUM_EXECUTABLE chromium)

if (NOT CHROMIUM_EXECUTABLE)
  message(FATAL_ERROR "chromium executable not found.")
endif()

function(f3d_js_test)
  cmake_parse_arguments(F3D_JS_TEST "USE_UTILS" "NAME;CANVAS_ID;INPUT_PATH" "" ${ARGN})

  # Set TEST_JS_MODULE and TEST_CANVAS_ID used in configure_file
  set(TEST_JS_MODULE "${CMAKE_CURRENT_SOURCE_DIR}/${F3D_JS_TEST_NAME}.js")
  set(TEST_CANVAS_ID "canvas")

  if (F3D_JS_TEST_CANVAS_ID)
    set(TEST_CANVAS_ID "${F3D_JS_TEST_CANVAS_ID}")
  endif()

  set(TEST_INPUT_URL "${F3D_SOURCE_DIR}/${F3D_JS_TEST_INPUT_PATH}")

  if (F3D_JS_TEST_USE_UTILS)
    set(_input_html "${CMAKE_CURRENT_SOURCE_DIR}/test_utils.html.in")
  else()
    set(_input_html "${CMAKE_CURRENT_SOURCE_DIR}/test.html.in")
  endif()

  configure_file(
    "${_input_html}"
    "${CMAKE_CURRENT_BINARY_DIR}/${F3D_JS_TEST_NAME}.html"
    @ONLY)

  add_test(NAME libf3d_bindings_js::${F3D_JS_TEST_NAME}
    COMMAND ${CHROMIUM_EXECUTABLE}
      --disable-application-cache
      --disable-extensions
      --disable-notifications
      --disable-restore-session-state
      --disable-gpu
      --enable-unsafe-swiftshader
      --new-window
      --no-default-browser-check
      --no-first-run
      --no-sandbox
      --headless=new
      --enable-logging=stderr
      --user-data-dir=${CMAKE_CURRENT_BINARY_DIR}/${F3D_JS_TEST_NAME}_data_dir
      --allow-file-access-from-files
      file://${CMAKE_CURRENT_BINARY_DIR}/${F3D_JS_TEST_NAME}.html
  )
  set_tests_properties(libf3d_bindings_js::${F3D_JS_TEST_NAME} PROPERTIES
    LABELS "libf3d;bindings;js"
    FAIL_REGULAR_EXPRESSION "F3D_ERROR"
  )
endfunction()

f3d_js_test(NAME "test_animation")
f3d_js_test(NAME "test_camera")
f3d_js_test(NAME "test_depth")
f3d_js_test(NAME "test_engine")
f3d_js_test(NAME "test_image")
f3d_js_test(NAME "test_interactor")
f3d_js_test(NAME "test_memory_view")
f3d_js_test(NAME "test_options")
f3d_js_test(NAME "test_render")
f3d_js_test(NAME "test_scene")
f3d_js_test(NAME "test_skinning")
f3d_js_test(NAME "test_splats")
f3d_js_test(NAME "test_custom_canvas" CANVAS_ID "custom_id")
f3d_js_test(NAME "test_statefile")
f3d_js_test(NAME "test_viewer_element" USE_UTILS INPUT_PATH "testing/data/cow.vtp")

if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260128)
  f3d_js_test(NAME "test_image_stream")
endif()
