## Tests that require custom usage of add_test and do not rely on f3d_test

# Test that f3d can try to read a system config file
add_test(NAME f3d::TestNoNoConfig COMMAND $<TARGET_FILE:f3d> --no-render)
set_tests_properties(f3d::TestNoNoConfig PROPERTIES TIMEOUT 20)

# Test invalid CLI args
add_test(NAME f3d::TestInvalidCLIArgs COMMAND $<TARGET_FILE:f3d> --up)
set_tests_properties(f3d::TestInvalidCLIArgs PROPERTIES PASS_REGULAR_EXPRESSION "Error parsing command line arguments")

# Test trying to load a plugin without specifying --plugins-path
add_test(NAME f3d::TestNoCliInvalidPlugin COMMAND $<TARGET_FILE:f3d> --load-plugins=invalid --no-render)
set_tests_properties(f3d::TestNoCliInvalidPlugin PROPERTIES PASS_REGULAR_EXPRESSION "Cannot open the library")

# Exercise the app-side relative-path branch of AugmentStatefileContent: a file group whose file lives
# in the statefile directory has its path stored relative to it. The file is copied next to the
# statefile so the relative branch is taken. This only asserts that saving succeeds; the produced path
# content is not inspected here (relative-path storage itself is asserted by the libf3d TestSDKStatefile
# test).
add_test(NAME f3d::TestStatefileRelativeAugmentSetup
  COMMAND ${CMAKE_COMMAND} -E copy ${F3D_SOURCE_DIR}/testing/data/cow.vtp ${CMAKE_BINARY_DIR}/Testing/Temporary/statefile_relative/cow.vtp)
set_tests_properties(f3d::TestStatefileRelativeAugmentSetup PROPERTIES FIXTURES_SETUP statefile_relative)
add_test(NAME f3d::TestStatefileRelativeAugment COMMAND $<TARGET_FILE:f3d> --no-render --no-config ${CMAKE_BINARY_DIR}/Testing/Temporary/statefile_relative/cow.vtp --save-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/statefile_relative/state.json --verbose)
set_tests_properties(f3d::TestStatefileRelativeAugment PROPERTIES FIXTURES_REQUIRED statefile_relative PASS_REGULAR_EXPRESSION "Statefile saved to")

# Window size in statefiles: save a statefile at a chosen resolution (a window is needed for the size
# to be captured), then load it back. These use add_test directly because f3d_test always injects
# --resolution, which takes precedence over the statefile window size.
add_test(NAME f3d::TestStatefileWindowSizeSave
  COMMAND $<TARGET_FILE:f3d> --no-config ${F3D_SOURCE_DIR}/testing/data/cow.vtp --resolution=512,384 --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestStatefileWindowSizeSave.png --save-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/TestStatefileWindowSize.json)
set_tests_properties(f3d::TestStatefileWindowSizeSave PROPERTIES PASS_REGULAR_EXPRESSION "Statefile saved to")

# Loading without --resolution restores the window size from the statefile
add_test(NAME f3d::TestStatefileWindowSizeLoad
  COMMAND $<TARGET_FILE:f3d> --no-config --load-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/TestStatefileWindowSize.json --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestStatefileWindowSizeLoad.png --verbose)
set_tests_properties(f3d::TestStatefileWindowSizeLoad PROPERTIES DEPENDS f3d::TestStatefileWindowSizeSave PASS_REGULAR_EXPRESSION "Window size set to 512x384")

# An explicit --resolution takes precedence: the statefile window size is not applied
add_test(NAME f3d::TestStatefileWindowSizeResolutionPrecedence
  COMMAND $<TARGET_FILE:f3d> --no-config --load-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/TestStatefileWindowSize.json --resolution=256,192 --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestStatefileWindowSizeResolutionPrecedence.png --verbose)
set_tests_properties(f3d::TestStatefileWindowSizeResolutionPrecedence PROPERTIES DEPENDS f3d::TestStatefileWindowSizeSave FAIL_REGULAR_EXPRESSION "Window size set to")

# Test that f3d resolution can be controlled from config file
add_test(NAME f3d::TestConfigResolution COMMAND $<TARGET_FILE:f3d> --config=${F3D_SOURCE_DIR}/testing/configs/resolution.json ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestConfigResolution.png --reference=${F3D_SOURCE_DIR}/testing/baselines/TestConfigResolution.png)

# Test filename template with multiple files
add_test(NAME f3d::TestMultiFileFileNameTemplate COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl ${F3D_SOURCE_DIR}/testing/data/dragon.vtu --output=${CMAKE_BINARY_DIR}/Testing/Temporary/{model.ext}.png --multi-file-mode=all --verbose)
set_tests_properties(f3d::TestMultiFileFileNameTemplate PROPERTIES PASS_REGULAR_EXPRESSION "multi_file.png")

# Test filename template with no files
add_test(NAME f3d::TestNoFileFileNameTemplate COMMAND $<TARGET_FILE:f3d> --output=${CMAKE_BINARY_DIR}/Testing/Temporary/{model.ext}.png --verbose)
set_tests_properties(f3d::TestNoFileFileNameTemplate PROPERTIES PASS_REGULAR_EXPRESSION "no_file.png")
set_tests_properties(f3d::TestNoFileFileNameTemplate PROPERTIES ENVIRONMENT "CTEST_F3D_NO_DATA_FORCE_RENDER=1")

# Test that f3d can read DPI scaling from system, note that this test has no reference baseline.
add_test(NAME f3d::TestSystemDPIScaling COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --dpi-aware --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestSystemDPIScaling.png)

if (APPLE)
  add_test(NAME f3d::TestAppleCmdMod COMMAND $<TARGET_FILE:f3d> --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestAppleCmdMod.png --reference=${F3D_SOURCE_DIR}/testing/baselines/TestAppleCmdMod.png --resolution=400,400)
  set_tests_properties(f3d::TestAppleCmdMod PROPERTIES ENVIRONMENT "CTEST_F3D_NO_DATA_FORCE_RENDER=1")
endif ()

# Test EGL/OSMesa load failure by loading fake libraries like libEGL.so located in ${F3D_SOURCE_DIR}/testing/data
if (UNIX AND NOT APPLE)
  add_test(NAME f3d::TestEGLLoadFailure COMMAND $<TARGET_FILE:f3d> --output=${CMAKE_BINARY_DIR}/Testing/Temporary/egl.png --rendering-backend=egl --verbose)
  set_tests_properties(f3d::TestEGLLoadFailure PROPERTIES PASS_REGULAR_EXPRESSION "Cannot find EGL library")
  set_tests_properties(f3d::TestEGLLoadFailure PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${F3D_SOURCE_DIR}/testing/data")

  add_test(NAME f3d::TestOSMesaLoadFailure COMMAND $<TARGET_FILE:f3d> --output=${CMAKE_BINARY_DIR}/Testing/Temporary/osmesa.png --rendering-backend=osmesa --verbose)
  set_tests_properties(f3d::TestOSMesaLoadFailure PROPERTIES PASS_REGULAR_EXPRESSION "Cannot find OSMesa library")
  set_tests_properties(f3d::TestOSMesaLoadFailure PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${F3D_SOURCE_DIR}/testing/data")
endif ()
