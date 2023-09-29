# Testing

F3D has an expansive suite of tests, you may want to run them locally,
either to validate your build or because you are contributing to F3D and want to add/modify a test.

## CMake Options

There is a few CMake options to F3D testing configuration:
* `BUILD_TESTING`: Enable the tests, off by default.
* `F3D_TESTING_ENABLE_RENDERING_TESTS`: An option to enable/disable test that require rendering capabilities, on by default.
* `F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS`: Certain tests can take some time to run, off by default, requires rendering tests.
* `F3D_TESTING_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON`: With VTK < 9.0.0, rendering can be very different, although not incorrect, so this option is provided, off by default.

## Running the tests

To run the tests, just use ctest from the build directory:

```
ctest
```

To run a specific test, use the -R ctest option:

```
ctest -R PLY
```

## Testing architecture

There is multiple layers of tests to ensure F3D test coverage is as high as possible
 - Application layer
 - Library layer
 - Bindings layer
 - VTK Extension layer

When contributing to F3D, it is necessary that any new code is covered by at least one layer
of test, but it could make sense to cover it with more if necessary.

### Application layer

All application test are just a command line run by ctest using the `f3d` executable.
ctest then check the output for any failure. Most of these tests are just using the `--output`
and `--ref` F3D option in order to check if a rendering behave correctly with specific options.

Everything is handled in `application/testing/CMakeLists.txt`.

Usually, adding a test is a simple as adding a line like this one:

```
f3d_test(NAME TestName DATA datafile.ext ARGS --args-to-test DEFAULT_LIGHTS)
```

 - `NAME` should be the name of the test, which must be unique
 - `DATA` should be a file in `testing/data` directory, though adding new file is possible
 - `ARGS` should be the F3D options to pass to the f3d executable, if any
 - `DEFAULT_LIGHTS` is expected when performing baselines comparison

Once the new test has been added, configure and build F3D, then run the test once:

```
ctest -R TestName
```

The test will fail but an image output will be generated in you build in `Testing/Temporary/TestName.png`.
Visually check that the generated file looks as expected, then add it to the F3D sources in `testing/baselines`.
Rerun the test, it should now pass.

### Library layer

When for some reason adding a test in the application layer is not possible, it is possible
to add a C++ test in the library layer. These tests are simple C++ methods that should return
`EXIT_SUCCESS` or `EXIT_FAILURE`. 

Everything is handled in `library/testing`.

To add a test, create a new `TestSDKName.cxx` file containing a `int TestSDKName(int argc, char* argv[])` method,
then implement your test in C++ using the [libf3d](../libf3d/README.md) API.
Then add you new file to `src/library/testing/CMakeLists.txt`.

It is supported to read file as input and perform image comparison against baselines as an output, see other tests as examples.

### Bindings layer

The libf3d supports multiple bindings, including Python, Java and Javascript.

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
