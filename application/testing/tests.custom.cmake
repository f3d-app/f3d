## Tests that require custom usage of add_test and do not rely on f3d_test

# Test that f3d can try to read a system config file
add_test(NAME f3d::TestNoNoConfig COMMAND $<TARGET_FILE:f3d> --no-render)
set_tests_properties(f3d::TestNoNoConfig PROPERTIES TIMEOUT 4)

# Test that we can try loading a plugin without F3D_PLUGINS_PATH being defined
add_test(NAME f3d::TestNoEnvInvalidPlugin COMMAND $<TARGET_FILE:f3d> --load-plugins=invalid --no-render)
set_tests_properties(f3d::TestNoEnvInvalidPlugin PROPERTIES PASS_REGULAR_EXPRESSION "Cannot open the library")

# Test invalid CLI args
add_test(NAME f3d::TestInvalidCLIArgs COMMAND $<TARGET_FILE:f3d> --up)
set_tests_properties(f3d::TestInvalidCLIArgs PROPERTIES PASS_REGULAR_EXPRESSION "Error parsing command line arguments")

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
