#[==[.md
# Adding a test using the f3d executable and compare rendering result against a baseline
# A resulting typical test command line would look like this:

```
f3d "/path/to/data.ext" "--no-config" "--resolution=300,300" "--output=/path/to/build/Testing/Temporary/TestName.png" "--reference=/path/to/source/testing/baselines/TestName.png" "--rendering-backend=auto"
```

Usage:

```
f3d_test(<NAME> [ARGS...])
```
  - `TONE_MAPPING` Marks that this test uses tone mapping which automatically
    disables it when using older version of VTK (<9.3.20240609)
  - `LONG_TIMEOUT` Marks a test to be enabled only if
    F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS is ON
  - `INTERACTION` If present, an interaction recording of the same name as the
    test will be played using `--interaction-test-play`. Such a recording
    should be cleaned up and long one should consider using LONG_TIMEOUT.
  - `INTERACTION_CONFIGURE` Same as `INTERACTION`, but the file will be configured
    with CMake first.
  - `INTERACTION_CONFIGURE` Same as `INTERACTION`, but the file will be configured
    with CMake first.
  - `NO_BASELINE` Mark the test to NOT compare its results against a baseline,
    usually used in conjunction with REGEXP
  - `NO_RENDER` Mark the test to NOT use rendering at all, through the `--no-render` CLI option
  - `NO_OUTPUT` Mark the test to NOT generate an output
  - `WILL_FAIL` Mark the test as expecting an actual failure, a success would mean the test had failed
  - `NO_DATA` Do not provide a data to the command line
  - `FORCE_RENDER` Position the environment variable `CTEST_F3D_NO_DATA_FORCE_RENDER` that forces a render
    even when F3D logic usually would not
  - `DPI_SCALE` Set the DPI scale through the environment variable `CTEST_F3D_FORCE_DPI_SCALE`, default is 1.0
  - `UI` Mark the test to require the presence of UI component and disable it otherwise
  - `PIPED` Mark the test to pipe the data (`cat data | f3d`) instead of providing the filename as data,
    doesn't work for external plugins, pass the reader as an arg, it will be used to force before VTK v9.6.20260128.
    Add `piped` test labels.
  - `SCRIPT` Mark the test to use a `--script` of the same name as the test
  - `NAME` Provide the name of the test, mandatory and must be unique
  - `CONFIG` Provide the `--config` to use, instead of `--no-config`
  - `RESOLUTION` Provide the `--resolution` to use, instead of `300,300`
  - `PLUGIN` Provide the `--load-plugins` to use, also set test labels accordingly
  - `THRESHOLD` Provide the `--reference-threshold` to use instead of the default
  - `REGEXP` Provide the regexp to check for in the stdout of the test, fails if not present
  - `REGEXP_FAIL` Provide the regexp to check for in the stdout of the test, fails if present
  - `HDRI` Provide the `--hdri-file` to use for this test
  - `RENDERING_BACKEND` Provide the `--rendering-backend` to use for this test instead of `auto`
  - `WORKING_DIR` Provide a specific working directory to use for this test instead of current dir
  - `LABELS` Provide a specific labels to identify and group tests
  - `DATA` Data to open, support multiple input, also set test labels accordingly
  - `DEPENDS` Tests the this test depends on if any
  - `ENV` Environment variables to set for this test
  - `ARGS` Supplement arguments to add to the f3d command line
#]==]

function(f3d_test)

  cmake_parse_arguments(F3D_TEST "TONE_MAPPING;LONG_TIMEOUT;INTERACTION;INTERACTION_CONFIGURE;NO_BASELINE;NO_RENDER;NO_OUTPUT;WILL_FAIL;NO_DATA_FORCE_RENDER;UI;SCRIPT" "NAME;CONFIG;RESOLUTION;THRESHOLD;REGEXP;REGEXP_FAIL;HDRI;RENDERING_BACKEND;WORKING_DIR;DPI_SCALE;PIPED;LABELS;PLUGIN" "DATA;DEPENDS;ENV;ARGS" ${ARGN})

  if(F3D_TEST_CONFIG)
    list(APPEND F3D_TEST_ARGS "--config=${F3D_TEST_CONFIG}")
  else()
    list(APPEND F3D_TEST_ARGS "--no-config")
  endif()

  set(_f3d_test_data)
  if (F3D_TEST_DATA)
    foreach(_single_data ${F3D_TEST_DATA})
      if(DEFINED f3d_INCLUDE_DIR)
        list(APPEND _f3d_test_data "${_single_data}")
      else()
        list(APPEND _f3d_test_data "${F3D_SOURCE_DIR}/testing/data/${_single_data}")
      endif()
      get_filename_component(FILE_EXT "${_single_data}" EXT)
      string(TOLOWER "${FILE_EXT}" FILE_EXT)
      string(REPLACE "." "" FILE_EXT "${FILE_EXT}")
      list(APPEND F3D_TEST_LABELS "${FILE_EXT}")
    endforeach()
  endif()

  if(F3D_TEST_INTERACTION)
    list(APPEND F3D_TEST_ARGS "--interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
  else ()
    if(F3D_TEST_INTERACTION_CONFIGURE)
      configure_file("${F3D_SOURCE_DIR}/testing/recordings/${F3D_TEST_NAME}.log.in" "${CMAKE_BINARY_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
      list(APPEND F3D_TEST_ARGS "--interaction-test-play=${CMAKE_BINARY_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
    endif()
  endif()

  if (F3D_TEST_HDRI)
    list(APPEND F3D_TEST_ARGS "--hdri-file=${F3D_SOURCE_DIR}/testing/data/${F3D_TEST_HDRI}" "--hdri-ambient" "--hdri-skybox")
    set(F3D_TEST_LONG_TIMEOUT ON)
  endif()

  if (F3D_TEST_SCRIPT)
    list(APPEND F3D_TEST_ARGS "--command-script=${F3D_SOURCE_DIR}/testing/scripts/${F3D_TEST_NAME}.txt")
  endif()

  if(F3D_TEST_NO_RENDER)
    list(APPEND F3D_TEST_ARGS "--no-render" "--verbose=debug")
  else()
    if(F3D_TEST_RESOLUTION)
      list(APPEND F3D_TEST_ARGS "--resolution=${F3D_TEST_RESOLUTION}")
    else()
      list(APPEND F3D_TEST_ARGS "--resolution=300,300")
    endif()

    if(NOT F3D_TEST_NO_OUTPUT)
      list(APPEND F3D_TEST_ARGS "--output=${CMAKE_BINARY_DIR}/Testing/Temporary/${F3D_TEST_NAME}.png")
    endif()
  endif()

  if(NOT F3D_TEST_NO_BASELINE)
    list(APPEND F3D_TEST_ARGS "--reference=${F3D_SOURCE_DIR}/testing/baselines/${F3D_TEST_NAME}.png")

    if(DEFINED F3D_TEST_THRESHOLD)
      list(APPEND F3D_TEST_ARGS "--reference-threshold=${F3D_TEST_THRESHOLD}")
    endif()
  endif()

  if(F3D_TEST_RENDERING_BACKEND)
    list(APPEND F3D_TEST_ARGS "--rendering-backend=${F3D_TEST_RENDERING_BACKEND}")
  else()
    # If no rendering backend is specified by the test, "auto" is used.
    # However, F3D_TESTING_FORCE_RENDERING_BACKEND can be used to force the rendering backend for these tests
    if(F3D_TESTING_FORCE_RENDERING_BACKEND)
      list(APPEND F3D_TEST_ARGS "--rendering-backend=${F3D_TESTING_FORCE_RENDERING_BACKEND}")
    endif()
  endif()

  if(F3D_TEST_PLUGIN)
    list(APPEND F3D_TEST_ARGS "--load-plugins=${F3D_TEST_PLUGIN}")
    set(F3D_TEST_LABELS "${F3D_TEST_LABELS};plugin;${F3D_TEST_PLUGIN}")
  endif()

  if(DEFINED f3d_INCLUDE_DIR)
    if (F3D_TEST_PIPED)
      message(FATAL_ERROR "PIPED test is not supported to external plugins")
    endif ()

    # Used for testing plugins
    find_package(f3d REQUIRED COMPONENTS application)
    set(_f3d_target "$<TARGET_FILE:f3d::f3d>")
  else()
    set(_f3d_target "$<TARGET_FILE:f3d>")
  endif()

  if (F3D_TEST_PIPED)
    list(APPEND F3D_TEST_LABELS "piped")
    if(VTK_VERSION VERSION_LESS 9.6.20260128)
      list(APPEND F3D_TEST_ARGS "--force-reader=${F3D_TEST_PIPED}")
    endif()
    list(JOIN F3D_TEST_ARGS " " F3D_TEST_ARGS_JOINED)
    add_test(
      NAME "f3d::${F3D_TEST_NAME}"
      COMMAND ${CMAKE_COMMAND}
        -DF3D_EXE:FILEPATH=${_f3d_target}
        -DF3D_PIPED_DATA=${_f3d_test_data}
        -DF3D_ARGS=${F3D_TEST_ARGS_JOINED}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/f3d_piped.cmake
        COMMAND_EXPAND_LISTS)
  else()
    add_test(NAME "f3d::${F3D_TEST_NAME}" COMMAND ${_f3d_target} ${_f3d_test_data} ${F3D_TEST_ARGS} COMMAND_EXPAND_LISTS)
  endif()

  if(F3D_TEST_LABELS)
    list(PREPEND F3D_TEST_LABELS "application")
    set_tests_properties("f3d::${F3D_TEST_NAME}" PROPERTIES
      LABELS "${F3D_TEST_LABELS}"
    )
  endif()

  set(_timeout "30")
  if(F3D_TEST_LONG_TIMEOUT)
    set(_timeout "120")
  endif()

  # sanitizer multipliers (multipliers are coming from ASan documentation)
  # "undefined" and "leak" have no overhead
  if(F3D_SANITIZER STREQUAL "address")
    math(EXPR _timeout "2 * ${_timeout}")
  endif()
  if(F3D_SANITIZER STREQUAL "thread")
    math(EXPR _timeout "15 * ${_timeout}")
  endif()
  if(F3D_SANITIZER STREQUAL "memory")
    math(EXPR _timeout "3 * ${_timeout}")
  endif()

  if(F3D_TEST_UI AND NOT F3D_MODULE_UI)
    # UI tests require ImGui
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
  endif()

  if(F3D_TEST_TONE_MAPPING AND VTK_VERSION VERSION_LESS 9.3.20240609)
    # After VTK 9.3.20240609, the tone mapping used in F3D is PBR Neutral
    # Testing tone mapping is now disabled because the reference image is different
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
  endif()

  if(NOT F3D_TESTING_ENABLE_RENDERING_TESTS)
    if(NOT F3D_TEST_NO_RENDER)
      set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
    endif()
  endif()
  if(NOT F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS)
    if(F3D_TEST_LONG_TIMEOUT)
      set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
    endif()
  endif()
  set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES TIMEOUT ${_timeout})

  if(F3D_TEST_WILL_FAIL)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES WILL_FAIL TRUE)
  endif()

  if(F3D_TEST_REGEXP)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "${F3D_TEST_REGEXP}")
  endif()

  if(F3D_TEST_REGEXP_FAIL)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES FAIL_REGULAR_EXPRESSION "${F3D_TEST_REGEXP_FAIL}")
  endif()

  if(F3D_TEST_DEPENDS)
    foreach(_single_depends ${F3D_TEST_DEPENDS})
      set_tests_properties(f3d::${_single_depends} PROPERTIES FIXTURES_SETUP f3d::${_single_depends}_FIXTURE)
      list(APPEND _depends_fixtures f3d::${_single_depends}_FIXTURE)
    endforeach()
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES FIXTURES_REQUIRED "${_depends_fixtures}")
  endif()

  if(F3D_TEST_WORKING_DIR)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES WORKING_DIRECTORY "${F3D_TEST_WORKING_DIR}")
  endif()

  set(f3d_test_env_vars ${F3D_TEST_ENV})
  list(APPEND f3d_test_env_vars "CTEST_F3D_PROGRESS_BAR=1")
  if (F3D_TEST_UI)
    list(APPEND f3d_test_env_vars "CTEST_F3D_CONSOLE_BADGE=1")
  endif ()
  if (F3D_TEST_NO_DATA_FORCE_RENDER)
    list(APPEND f3d_test_env_vars "CTEST_F3D_NO_DATA_FORCE_RENDER=1")
  endif ()
  if (F3D_TEST_DPI_SCALE)
    list(APPEND f3d_test_env_vars "CTEST_F3D_FORCE_DPI_SCALE=${F3D_TEST_DPI_SCALE}")
  else()
    list(APPEND f3d_test_env_vars "CTEST_F3D_FORCE_DPI_SCALE=1.0")
  endif ()

  set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES ENVIRONMENT
    "F3D_PLUGINS_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${f3d_test_env_vars}")

endfunction()
