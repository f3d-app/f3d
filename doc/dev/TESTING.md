# Testing

F3D has an expansive suite of tests, that you may want to run locally,
either to validate your build or because you are contributing to F3D and want to add/modify a test.
To enable the tests, run `cmake -DBUILD_TESTING=ON ../` from the `build`  directory.

## CMake Options

There are a few CMake options to configure the F3D testing framework:
* `F3D_TESTING_ENABLE_RENDERING_TESTS`: An option to enable/disable test that require rendering capabilities, on by default.
* `F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS`: Certain tests can take some time to run, off by default, requires rendering tests.
* `F3D_TESTING_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON`: With VTK < 9.0.0, rendering can be very different, although not incorrect, so this option is provided, off by default.

## Running the tests

To run all tests, build the `f3d` binary and run `ctest` from the build directory:

```
make
ctest
```

To run a specific test, use the `ctest -R <testname>` option:

```
ctest -R PLY
```

## Testing architecture

There are multiple layers of tests to ensure that test coverage is as high as possible
 - Application layer
 - Library layer
 - Bindings layer
 - VTK Extension layer

When contributing to F3D, it is necessary that any new code is covered by a test in at least one layer above. Additional tests can be created in other layers, depending on the added functionality / bugfix.

### Application layer

All application tests are initiated via the command line run by `ctest` and use the built`f3d` executable.
`ctest` then compares the rendered output with the expected (baseline) output for differences.
Most of the tests use the `--output` (which saves the rendered output to a file) and `--ref` 
(which compares the output to a reference file) F3D options in order to identify differences.

The application test framework is handled in `application/testing/CMakeLists.txt`.

Usually, adding a test is as simple as adding a line like this one:

```
f3d_test(NAME TestName DATA datafile.ext ARGS --args-to-test DEFAULT_LIGHTS)
```

 where
 - `NAME` should be the name of the test, which must be unique
 - `DATA` should be a file in `testing/data` directory, though adding a new file is possible
 - `ARGS` should be the F3D options to pass to the f3d executable, if any
 - `DEFAULT_LIGHTS` is expected when performing baselines comparison

Once the new test has been added, configure and build F3D, then run the test (-VV for verbose output):

```
ctest -R TestName -VV
```

The test will fail but an image output will be generated in the build directory, namely `build/Testing/Temporary/TestName.png`.
Visually check that the generated file looks as expected, then add it to the F3D sources in `testing/baselines`.
Rerun the test, it should now pass.

### Recovering baselines from CI
Occassionally you may need to recover a baseline from the CI, becuase you can not generate the baseline image on your local environment. If this is required, create a PR and let the test run (fail) on CI. Check the actions run summary on Github and download the appropriate `baseline` archive based on your OS. Extract it and navigate to the `build/Testing/Temporary/TestName.png`. Extract this file and add it to the `testing/baselines` directory.

### Creating Interaction tests
Sometimes you contribute changes that affect how the end user interacts with F3D, example toggling orthographic projection on/off or zooming in or out of the rendered image. These are simulated by interaction tests. F3D has the functionality to record 
any mouse wheel scrolls, mouse movements as well as keypresses to a file. This is enabled by running 

```
f3d --interaction-test-record ./TestName.log
```

where
- `TestName` should be the name of the test case. `TestName.log` should be added to the `./testing/recordings` directory.

Care should be taken to perform the minimum number of events to simulate the interaction. For instance, if you want to simulate 
the number `5` being pressed, there should be no mouse events or other keypress events. To save the file, exit `f3d`

You can verify that your interaction test file is correct by running

```
f3d --interaction-test-play ./TestName.log
```

After you have verififed that the interaction file works as expected, copy it to `./testing/recordings` and create the interaction test case by adding the below line to `application/testing/CMakeLists.txt`

```
f3d_test(NAME TestName DATA datafile.ext INTERACTION DEFAULT_LIGHTS)
```

 where
 - `NAME` should be the name of the test, which must be unique
 - `DATA` should be a file in `testing/data` directory, though adding a new file is possible
 - `INTERACTION` signifies that this is an interaction test
 - `DEFAULT_LIGHTS` is expected when performing baselines comparison

Once the new test has been added, configure and build F3D, then run the test (-VV for verbose output):

```
ctest -R TestName -VV
```

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

The libf3d supports multiple bindings, including C, Python, Java and Javascript.

When improving/modifying these bindings, it is necessary to also improve/modify the bindings tests accordingly.
Please take a look into `<bindings>/testing` for examples to follow.

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
