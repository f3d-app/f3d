list(APPEND libf3dSDKTests_list
     TestSDKAnimation.cxx
     TestSDKCamera.cxx
     TestSDKCompareWithFile.cxx
     TestSDKDropZone.cxx
     TestSDKDynamicBackgroundColor.cxx
     TestSDKDynamicFontFile.cxx
     TestSDKDynamicLightIntensity.cxx
     TestSDKDynamicProperties.cxx
     TestSDKEngine.cxx
     TestSDKEngineExceptions.cxx
     TestSDKImage.cxx
     TestSDKInteractorCallBack.cxx
     TestSDKInteractorDropFullScene.cxx
     TestSDKLoadFromMemory.cxx
     TestSDKLoader.cxx
     TestSDKLog.cxx
     TestSDKMultiColoring.cxx
     TestSDKMultiOptions.cxx
     TestSDKOptions.cxx
     TestSDKOptionsIO.cxx
     TestSDKRenderAndInteract.cxx
     TestSDKRenderFinalShader.cxx
     TestSDKWindowNative.cxx
     TestPseudoUnitTest.cxx
)

# HDRI test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9767
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  list(APPEND libf3dSDKTests_list
    TestSDKDynamicHDRI.cxx
    )
endif()

# Invalid loader codepath needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11287
# Merge with TestSDKLoader.cxx when VTK 9.4.0 is required.
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  list(APPEND libf3dSDKTests_list
    TestSDKLoaderInvalid.cxx
    )
endif()

# Configure the log file for dropfile test
configure_file("${F3D_SOURCE_DIR}/testing/recordings/TestSDKInteractorCallBack.log.in"
               "${CMAKE_BINARY_DIR}/TestSDKInteractorCallBack.log") # Dragon.vtu; S
configure_file("${F3D_SOURCE_DIR}/testing/recordings/TestSDKInteractorDropFullScene.log.in"
               "${CMAKE_BINARY_DIR}/TestSDKInteractorDropFullScene.log") # world.obj; S

# External window tests
# TODO better control of deps
if(F3D_MODULE_EXTERNAL_RENDERING AND F3D_TESTING_ENABLE_ONSCREEN_TESTS)
  find_package(glfw3)
  if(glfw3_FOUND)
    list(APPEND libf3dSDKTests_list
        TestSDKExternalWindowGLFW.cxx
        TestSDKExternalWindowGLX.cxx
        )
  endif()
  find_package(Qt5 COMPONENTS OpenGL)
  if(Qt5_FOUND)
    list(APPEND libf3dSDKTests_list
        TestSDKExternalWindowQT.cxx
        )
  endif()
endif()

# create the testing file and list of tests
# CMake variables are set to work around this issue:
# https://gitlab.kitware.com/cmake/cmake/-/issues/21049
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "f3d::engine::autoloadPlugins();")
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "")
set(CMAKE_TESTDRIVER_ARGVC_FUNCTION "")
create_test_sourcelist(_libf3dSDKTests libf3dSDKTests.cxx ${libf3dSDKTests_list} EXTRA_INCLUDE engine.h)

# add the executable
add_executable(libf3dSDKTests ${_libf3dSDKTests})
set_target_properties(libf3dSDKTests PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  )

target_compile_definitions(libf3dSDKTests PRIVATE F3D_MODULE_EXR=$<BOOL:${F3D_MODULE_EXR}>)
target_compile_definitions(libf3dSDKTests PRIVATE F3D_DISABLE_CXX17_API)

# Remove this once VTK 9.3 support is removed
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240729)
  target_compile_definitions(libf3dSDKTests PRIVATE F3D_SSIM_COMPARE)
endif()

# List tests that do not require rendering
list(APPEND libf3dSDKTestsNoRender_list
     TestSDKEngineExceptions
     TestSDKOptions
     TestSDKOptionsIO
     TestSDKLog
     TestSDKLoader)

# Add all the ADD_TEST for each test
foreach (test ${libf3dSDKTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME libf3d::${TName} COMMAND libf3dSDKTests ${TName}  "${F3D_SOURCE_DIR}/testing/" "${CMAKE_BINARY_DIR}/Testing/Temporary/" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set_tests_properties(libf3d::${TName} PROPERTIES TIMEOUT 30)
  if (NOT F3D_TESTING_ENABLE_RENDERING_TESTS AND NOT ${TName} IN_LIST libf3dSDKTestsNoRender_list)
    set_tests_properties(libf3d::${TName} PROPERTIES DISABLED ON)
  endif ()
endforeach ()

# Specific test properties to check
# CTest can't check colored output as it contains special chars
set_tests_properties(libf3d::TestSDKLog PROPERTIES PASS_REGULAR_EXPRESSION
"\
Test Info\nTest Warning\nTest Error\n\
Test Info\nTest Warning\nTest Error\n\
Test Warning\nTest Error\nTest Error\n\
Test Print Debug\nTest Print Info\nTest Print Warning\nTest Print Error\n\
Test Debug Coloring")

if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  set_tests_properties(libf3d::TestSDKDynamicHDRI PROPERTIES TIMEOUT 90)
  if(NOT F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS)
    set_tests_properties(libf3d::TestSDKDynamicHDRI PROPERTIES DISABLED ON)
  endif()
endif()

target_link_libraries(libf3dSDKTests libf3d)

if(glfw3_FOUND)
  # external window test using glfw
  target_link_libraries(libf3dSDKTests glfw OpenGL::GLX X11)
endif()

if(Qt5_FOUND)
  # external window test using QT
  target_link_libraries(libf3dSDKTests Qt5::OpenGL)
endif()

# make sure the libf3d API is compatible with the right C++ standard
set_target_properties(libf3dSDKTests PROPERTIES CXX_STANDARD 11)

# Add Cxx17 tests
list(APPEND libf3dSDKCxx17Tests_list
     TestSDKOptionsCxx17.cxx
)

# create the testing file and list of tests
create_test_sourcelist(_libf3dSDKCxx17Tests libf3dSDKCxx17Tests.cxx ${libf3dSDKCxx17Tests_list})

# add the executable
add_executable(libf3dSDKCxx17Tests ${_libf3dSDKCxx17Tests})
set_target_properties(libf3dSDKCxx17Tests PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  )

# Add all the ADD_TEST for each test
foreach (test ${libf3dSDKCxx17Tests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME libf3d::${TName} COMMAND libf3dSDKCxx17Tests ${TName}  "${F3D_SOURCE_DIR}/testing/" "${CMAKE_BINARY_DIR}/Testing/Temporary/" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set_tests_properties(libf3d::${TName} PROPERTIES TIMEOUT 30)
endforeach ()

target_link_libraries(libf3dSDKCxx17Tests libf3d)

# make sure the libf3d API is compatible with the right C++ standard
set_target_properties(libf3dSDKCxx17Tests PROPERTIES CXX_STANDARD 17)
