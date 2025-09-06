set(f3djsTests_list
  "test_options"
)

find_program(CHROMIUM_EXECUTABLE chromium)

if (NOT CHROMIUM_EXECUTABLE)
  message(FATAL_ERROR "chromium executable not found.")
endif()

# Add all the ADD_TEST for each test
foreach(test_name ${f3djsTests_list})

  set(TEST_JS_MODULE "${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.js")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/test.html.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${test_name}.html"
    @ONLY)

  add_test(NAME f3djs::${test_name}
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
      --user-data-dir=${CMAKE_CURRENT_BINARY_DIR}/${test_name}_data_dir
      --allow-file-access-from-files
      file://${CMAKE_CURRENT_BINARY_DIR}/${test_name}.html
  )

  set_tests_properties(f3djs::${test_name} PROPERTIES
    FAIL_REGULAR_EXPRESSION "F3D_ERROR")
endforeach()
