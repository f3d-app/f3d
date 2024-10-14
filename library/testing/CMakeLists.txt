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
     TestSDKSceneFromMemory.cxx
     TestSDKScene.cxx
     TestSDKLog.cxx
     TestSDKMultiColoring.cxx
     TestSDKMultiOptions.cxx
     TestSDKOptions.cxx
     TestSDKOptionsIO.cxx
     TestSDKRenderAndInteract.cxx
     TestSDKRenderFinalShader.cxx
     TestSDKWindowAuto.cxx
     TestPseudoUnitTest.cxx
)

# HDRI test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9767
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  list(APPEND libf3dSDKTests_list
    TestSDKDynamicHDRI.cxx
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
cmake_dependent_option(F3D_TESTING_ENABLE_EXTERNAL_GLFW "Test external GLFW" OFF "F3D_MODULE_EXTERNAL_RENDERING" OFF)
cmake_dependent_option(F3D_TESTING_ENABLE_EXTERNAL_QT "Test external QT" OFF "F3D_MODULE_EXTERNAL_RENDERING" OFF)
cmake_dependent_option(F3D_TESTING_ENABLE_EXTERNAL_OSMESA "Test external OSMesa" OFF "F3D_MODULE_EXTERNAL_RENDERING" OFF)
cmake_dependent_option(F3D_TESTING_ENABLE_EXTERNAL_EGL "Test external GLX" OFF "F3D_MODULE_EXTERNAL_RENDERING" OFF)

if(F3D_MODULE_EXTERNAL_RENDERING)
  if(F3D_TESTING_ENABLE_EXTERNAL_GLFW)
    find_package(glfw3 REQUIRED)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowGLFW.cxx)
  endif()
  if(F3D_TESTING_ENABLE_EXTERNAL_QT)
    find_package(Qt5 COMPONENTS OpenGL REQUIRED)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowQT.cxx)
  endif()
  if(F3D_TESTING_ENABLE_EXTERNAL_OSMESA)
    find_library(OSMesa_LIB OSMesa REQUIRED)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowOSMesa.cxx)
  endif()
  if(F3D_TESTING_ENABLE_GLX_TESTS)
    find_package(OpenGL COMPONENTS GLX REQUIRED)
    find_library(X11_LIB X11 REQUIRED)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowGLX.cxx)
  endif()
  if(F3D_TESTING_ENABLE_EXTERNAL_EGL)
    find_package(OpenGL COMPONENTS EGL REQUIRED)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowEGL.cxx)
  endif()
  if(WIN32)
    find_package(OpenGL COMPONENTS OpenGL REQUIRED)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowWGL.cxx)
  endif()
  if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    list(APPEND libf3dSDKTests_list TestSDKExternalWindowCOCOA.mm)
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

if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  set_tests_properties(libf3d::TestSDKDynamicHDRI PROPERTIES TIMEOUT 90)
  if(NOT F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS)
    set_tests_properties(libf3d::TestSDKDynamicHDRI PROPERTIES DISABLED ON)
  endif()
endif()

target_link_libraries(libf3dSDKTests libf3d)

if(F3D_TESTING_ENABLE_EXTERNAL_GLFW)
  # external window test using glfw
  target_link_libraries(libf3dSDKTests glfw)
endif()

if(F3D_TESTING_ENABLE_EXTERNAL_QT)
  # external window test using QT
  target_link_libraries(libf3dSDKTests Qt5::OpenGL)
endif()

if(F3D_TESTING_ENABLE_EXTERNAL_OSMESA)
  # external window test using OSMesa
  target_link_libraries(libf3dSDKTests "${OSMesa_LIB}")
endif()

if(F3D_MODULE_EXTERNAL_RENDERING AND F3D_TESTING_ENABLE_GLX_TESTS)
  # external window test using GLX
  target_link_libraries(libf3dSDKTests OpenGL::GLX "${X11_LIB}")
endif()

if(F3D_TESTING_ENABLE_EXTERNAL_EGL)
  # external window test using EGL
  target_link_libraries(libf3dSDKTests OpenGL::EGL)
endif()

if(F3D_MODULE_EXTERNAL_RENDERING AND WIN32)
  # external window test using WGL
  target_link_libraries(libf3dSDKTests OpenGL::GL)
endif()

if(F3D_MODULE_EXTERNAL_RENDERING AND APPLE)
  target_link_libraries(libf3dSDKTests ${COCOA_LIBRARY} ${OpenGL_LIBRARY})
endif()

# make sure the libf3d API is compatible with the right C++ standard
set_target_properties(libf3dSDKTests PROPERTIES CXX_STANDARD 17)
