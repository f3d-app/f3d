# Testing

F3D has an extensive suite of tests that can be run locally, either to validate your build or because you are contributing to F3D and want to add/modify a test.

## CMake Options

There are a few CMake options to configure the F3D testing framework:
* `BUILD_TESTING`: Enable the test framework, off by default.
* `F3D_TESTING_ENABLE_RENDERING_TESTS`: An option to enable/disable test that require rendering capabilities, on by default.
* `F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS`: Certain tests can take some time to run, off by default, requires rendering tests.
* `F3D_TESTING_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON`: With VTK < 9.1.0, rendering can be very different, although not incorrect, so this option is provided, off by default.

## Running the tests

To run all tests, build and then run `ctest` from the build directory:

```
cmake --build .
ctest
```

To run a specific test, use the `ctest -R <testname>` option:

```
ctest -R PLY
```

## Testing architecture

There are multiple layers of tests to ensure that testing covers all aspects of the application. The layers of the application are
 - Application layer
 - Library layer
 - Bindings layer
 - VTK Extension layer

When contributing to F3D, it is necessary that new code is covered by a test in at least one layer above. Additional tests can be created in other layers, depending on the change.

### Application layer

All application tests are initiated via the `CTest` (https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html) testing framework.
`CTest` runs `f3d` with the `--output` and `--ref` arguments which renders an output image and compares it to a reference image. F3D compares the differences between the 2 images and generates a diff file if there are differences between the images.

All aspects of the application test framework are handled in `application/testing/CMakeLists.txt`.

Usually, adding a test is as simple as adding a line like this one:

```
f3d_test(NAME TestName DATA datafile.ext ARGS --args-to-test DEFAULT_LIGHTS)
```

 where
 - `NAME` should be the name of the test, which must be unique
 - `DATA` should be a file in `testing/data` directory, though adding a new file is possible
 - `ARGS` should be the F3D options to pass to the f3d executable, if any
 - `DEFAULT_LIGHTS` is expected when performing baselines comparison

Once the new test has been added, configure and build F3D, then run the test (`-VV` for verbose output):

```
ctest -R TestName -VV
```

The test will run and fail but an image output will be generated in the build directory, namely `Testing/Temporary/TestName.png`.
Visually check that the generated file looks as expected, then add it to the F3D sources in `testing/baselines`.
Rerun the test, it should now pass.

### Recovering baselines from CI
Occasionally you may need to recover a baseline from the CI. If this is required, create a PR and let the test run and fail on CI. Check the actions run summary on Github and download the appropriate `baseline` archive. Extract the archive and navigate to the `build/Testing/Temporary/TestName.png`. Visually check that the generated file looks as expected, then add it to the F3D sources in `testing/baselines`.

### Creating Interaction tests
Sometimes you may contribute changes that affect how the end user interacts with F3D, example, toggling orthographic projection on/off or zooming in/out of the rendered image. These human interactions are simulated by interaction tests. F3D has the functionality to record human interactions such as mouse wheel scrolls, mouse movements as well as keypresses to a file. This functionality is enabled by running 

```
f3d --interaction-test-record ./TestName.log
```

where
- `TestName` should be the name of the test case.

While recording the interaction, care should be taken to perform the minimum number of events to simulate the interaction. For example, if you want to simulate the number `5` being pressed, there should be no mouse events or other keypress events during the recording of the interaction. The interaction file is saved when `f3d` is exited.

You can verify that your interaction file is correct by playing back the interaction

```
f3d --interaction-test-play ./TestName.log
```

Interactions that are not needed for the test can be removed manually. After verifying that the interaction file works as expected, copy it to `./testing/recordings`. The name of the interaction file should be the same as the test name. The interaction test case can then be created by adding 

```
f3d_test(NAME TestName DATA datafile.ext INTERACTION DEFAULT_LIGHTS)
```

to `application/testing/CMakeLists.txt`

 where
 - `INTERACTION` signifies that this is an interaction test

The steps to running the test are the same as above.

### Library layer

When for some reason adding a test in the application layer is not possible, it is possible
to add a C++ test in the library layer. These tests are simple C++ methods that should return
`EXIT_SUCCESS` or `EXIT_FAILURE`. 

Library test cases are handled in `library/testing`.

To add a test, create a new `TestSDKName.cxx` file containing a `int TestSDKName(int argc, char* argv[])` method,
then implement your test in C++ using the [libf3d](../libf3d/README.md) API.
Then add you new file to `src/library/testing/CMakeLists.txt`.

It is supported to read file as input and perform image comparison against baselines as an output, see other tests as examples.

### Bindings layer

The libf3d supports multiple bindings, including Python, Java and Javascript.

When improving/modifying these bindings, it is necessary to also improve/modify the bindings tests accordingly.
Please take a look into `<java/python/webassembly>/testing` for examples to follow.

### VTKExtensions layer

When for some reason adding a test in the application or library layer is not possible, it is possible
to add a C++ test in the VTKExtensions layer. These tests are simple C++ methods that should return
`EXIT_SUCESS` or `EXIT_FAILURE`. 

Everything is handled in `library/VTKExtensions/ModuleName/Testing`.

To add a test, first identify which VTKExtensions module you need to add a test into, 
then create a new `TestName.cxx` file containing a `int TestName(int argc, char* argv[])` method,
then implement your test in C++ using VTK and F3D VTKExtensions modules.
Then add you new file to `library/VTKExtensions/ModuleName/Testing/CMakeLists.txt`.

It is supported to read file as input if needed, see other tests as examples.
