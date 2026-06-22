## Tests that require custom usage of add_test and do not rely on f3d_test

# Test that f3d can try to read a system config file
add_test(NAME f3d::TestNoNoConfig COMMAND $<TARGET_FILE:f3d> --no-render)
set_tests_properties(f3d::TestNoNoConfig PROPERTIES TIMEOUT 4)

# Test invalid CLI args
add_test(NAME f3d::TestInvalidCLIArgs COMMAND $<TARGET_FILE:f3d> --up)
set_tests_properties(f3d::TestInvalidCLIArgs PROPERTIES PASS_REGULAR_EXPRESSION "Error parsing command line arguments")

# Test trying to load a plugin without specifying --plugins-path
add_test(NAME f3d::TestNoCliInvalidPlugin COMMAND $<TARGET_FILE:f3d> --load-plugins=invalid --no-render)
set_tests_properties(f3d::TestNoCliInvalidPlugin PROPERTIES PASS_REGULAR_EXPRESSION "Cannot open the library")

# Test that f3d can save a statefile
add_test(NAME f3d::TestSaveStatefile COMMAND $<TARGET_FILE:f3d> --no-render --no-config -D render.background.color=0,1,0 ${F3D_SOURCE_DIR}/testing/data/cow.vtp --save-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/TestSaveStatefile.json --verbose)
set_tests_properties(f3d::TestSaveStatefile PROPERTIES PASS_REGULAR_EXPRESSION "Statefile saved to")

# Test that `-` writes the statefile to the standard output instead of the clipboard
add_test(NAME f3d::TestSaveStatefileStdout COMMAND $<TARGET_FILE:f3d> --no-render --no-config ${F3D_SOURCE_DIR}/testing/data/cow.vtp --save-statefile=-)
set_tests_properties(f3d::TestSaveStatefileStdout PROPERTIES PASS_REGULAR_EXPRESSION "\"options\"")

# Test that missing parent directories of the statefile path are created
add_test(NAME f3d::TestSaveStatefileCreatesDir COMMAND $<TARGET_FILE:f3d> --no-render --no-config ${F3D_SOURCE_DIR}/testing/data/cow.vtp --save-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/TestSaveStatefileCreatesDir/sub/cow.json --verbose)
set_tests_properties(f3d::TestSaveStatefileCreatesDir PROPERTIES PASS_REGULAR_EXPRESSION "Statefile saved to")

# Test that f3d can load a statefile, applying its options and files
add_test(NAME f3d::TestLoadStatefile COMMAND $<TARGET_FILE:f3d> --no-render --no-config --load-statefile=${F3D_SOURCE_DIR}/testing/configs/test_statefile.json --verbose)
set_tests_properties(f3d::TestLoadStatefile PROPERTIES PASS_REGULAR_EXPRESSION "from statefile options")

# Test that command line options take precedence over a loaded statefile
add_test(NAME f3d::TestLoadStatefileOverride COMMAND $<TARGET_FILE:f3d> --no-render --no-config --load-statefile=${F3D_SOURCE_DIR}/testing/configs/test_statefile.json -D render.background.color=1,0,0 --verbose)
set_tests_properties(f3d::TestLoadStatefileOverride PROPERTIES PASS_REGULAR_EXPRESSION "'render.background.color' = '1,0,0' from CLI options")

# Test that a missing statefile is skipped with a warning instead of failing
add_test(NAME f3d::TestLoadStatefileMissing COMMAND $<TARGET_FILE:f3d> --no-render --no-config --load-statefile=${CMAKE_BINARY_DIR}/Testing/Temporary/does_not_exist.json ${F3D_SOURCE_DIR}/testing/data/cow.vtp --verbose)
set_tests_properties(f3d::TestLoadStatefileMissing PROPERTIES PASS_REGULAR_EXPRESSION "Could not open statefile, skipping")

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
