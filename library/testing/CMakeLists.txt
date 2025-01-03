list(APPEND libf3dSDKTests_list
     TestSDKAnimation.cxx
     TestSDKCamera.cxx
     TestSDKCompareWithFile.cxx
     TestSDKDropZone.cxx
     TestSDKDynamicLightIntensity.cxx
     TestSDKEngine.cxx
     TestSDKEngineExceptions.cxx
     TestSDKImage.cxx
     TestSDKInteractorDropFullScene.cxx
     TestSDKInteractorCommand.cxx
     TestSDKSceneFromMemory.cxx
     TestSDKScene.cxx
     TestSDKLog.cxx
     TestSDKMultiColoring.cxx
     TestSDKOptions.cxx
     TestSDKOptionsIO.cxx
     TestSDKReaderSelection.cxx
     TestSDKRenderFinalShader.cxx
     TestSDKUtils.cxx
     TestSDKWindowAuto.cxx
     TestPseudoUnitTest.cxx
)

# Tests having UI widgets
if(F3D_MODULE_UI)
  list(APPEND libf3dSDKTests_list
    TestSDKConsole.cxx
    TestSDKDynamicBackgroundColor.cxx
    TestSDKDynamicFontFile.cxx
    TestSDKDynamicProperties.cxx
    TestSDKInteractorCallBack.cxx
    TestSDKInteractorDocumentation.cxx
    TestSDKMultiOptions.cxx
    )

  # HDRI test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9767
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
    list(APPEND libf3dSDKTests_list
      TestSDKDynamicHDRI.cxx
      )
  endif()
endif()

# These tests needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10155
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20230426)
  list(APPEND libf3dSDKTests_list
    TestSDKRenderAndInteract.cxx
    TestSDKStartInteractor.cxx
    )
endif()

if (VTK_VERSION VERSION_GREATER 9.2.6)
    list(APPEND libf3dSDKTests_list
        TestSDKEngineRecreation.cxx
        )
endif()

# Invalid scene codepath needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11287
# Merge with TestSDKScene.cxx when VTK 9.4.0 is required.
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  list(APPEND libf3dSDKTests_list
    TestSDKSceneInvalid.cxx
    )
endif()

# Configure the log file for dropfile test
configure_file("${F3D_SOURCE_DIR}/testing/recordings/TestSDKInteractorCallBack.log.in"
               "${CMAKE_BINARY_DIR}/TestSDKInteractorCallBack.log") # Dragon.vtu; S
configure_file("${F3D_SOURCE_DIR}/testing/recordings/TestSDKInteractorDropFullScene.log.in"
               "${CMAKE_BINARY_DIR}/TestSDKInteractorDropFullScene.log") # world.obj; S

# External window tests
option(F3D_TESTING_ENABLE_EXTERNAL_GLFW "Test external GLFW" OFF)
option(F3D_TESTING_ENABLE_EXTERNAL_QT "Test external QT" OFF)

set(libf3dSDKTests_link_libs "")

if(F3D_TESTING_ENABLE_EXTERNAL_GLFW)
  find_package(glfw3 REQUIRED)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowGLFW.cxx)
  list(APPEND libf3dSDKTests_link_libs glfw)
endif()
if(F3D_TESTING_ENABLE_EXTERNAL_QT)
  find_package(Qt5 COMPONENTS OpenGL REQUIRED)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowQT.cxx)
  list(APPEND libf3dSDKTests_link_libs Qt5::OpenGL)
endif()
if(F3D_TESTING_ENABLE_OSMESA_TESTS AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914)
  find_library(OSMesa_LIB OSMesa REQUIRED)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowOSMesa.cxx)
  list(APPEND libf3dSDKTests_link_libs "${OSMesa_LIB}")
endif()
if(F3D_TESTING_ENABLE_GLX_TESTS AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914)
  find_package(OpenGL COMPONENTS GLX REQUIRED)
  find_library(X11_LIB X11 REQUIRED)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowGLX.cxx)
  list(APPEND libf3dSDKTests_link_libs OpenGL::GLX "${X11_LIB}")
endif()
if(F3D_TESTING_ENABLE_EGL_TESTS AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914)
  find_package(OpenGL COMPONENTS EGL REQUIRED)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowEGL.cxx)
  list(APPEND libf3dSDKTests_link_libs OpenGL::EGL)
endif()
if(WIN32)
  find_package(OpenGL COMPONENTS OpenGL REQUIRED)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowWGL.cxx)
  list(APPEND libf3dSDKTests_link_libs OpenGL::GL)
endif()
if(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(OpenGL_LIBRARY OpenGL)
  list(APPEND libf3dSDKTests_list TestSDKExternalWindowCOCOA.mm)
  list(APPEND libf3dSDKTests_link_libs "${COCOA_LIBRARY}" "${OpenGL_LIBRARY}")
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

# Remove this once VTK 9.3 support is removed
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240729)
  target_compile_definitions(libf3dSDKTests PRIVATE F3D_SSIM_COMPARE)
endif()

if (F3D_MODULE_EXR)
  target_compile_definitions(libf3dSDKTests PRIVATE F3D_MODULE_EXR)
endif ()

# List tests that do not require rendering
list(APPEND libf3dSDKTestsNoRender_list
     TestSDKEngineExceptions
     TestSDKOptions
     TestSDKOptionsIO
     TestSDKLog
     TestSDKScene)

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

if(F3D_MODULE_UI AND VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  set_tests_properties(libf3d::TestSDKDynamicHDRI PROPERTIES TIMEOUT 120)
  if(NOT F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS)
    set_tests_properties(libf3d::TestSDKDynamicHDRI PROPERTIES DISABLED ON)
  endif()
endif()

if(F3D_MODULE_UI AND NOT F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS)
  set_tests_properties(libf3d::TestSDKInteractorCallBack PROPERTIES DISABLED ON)
endif()

target_link_libraries(libf3dSDKTests libf3d ${libf3dSDKTests_link_libs})

# make sure the libf3d API is compatible with the right C++ standard
set_target_properties(libf3dSDKTests PROPERTIES CXX_STANDARD 17)
