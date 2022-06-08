list(APPEND libf3dSDKTests_list
     TestSDKRenderAndInteract.cxx
     TestSDKEngine.cxx
     TestSDKEngineExceptions.cxx
     TestSDKOptions.cxx
     TestSDKLog.cxx
     TestSDKLoader.cxx
    )

# Test image comparison only with VTK > 9.0.1
if(VTK_VERSION VERSION_GREATER 9.0.1)
  list(APPEND libf3dSDKTests_list
    TestSDKCompareWithFile.cxx
    TestSDKImage.cxx
    TestSDKInteractionDirectory.cxx
    TestSDKWindowNative.cxx
    )
endif()

# Animation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
if(VTK_VERSION VERSION_GREATER_EQUAL 9.0.20201016)
  list(APPEND libf3dSDKTests_list
    TestSDKAnimation.cxx
    )
endif()

# Drop file test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9199
if(VTK_VERSION VERSION_GREATER_EQUAL 9.1.20220519)
  list(APPEND libf3dSDKTests_list
    TestSDKInteractorCallBack.cxx
    )
endif()

# Configure the log file for dropfile test
configure_file("${CMAKE_SOURCE_DIR}/testing/recordings/TestSDKInteractorCallBack.log.in"
               "${CMAKE_BINARY_DIR}/TestSDKInteractorCallBack.log") # Dragon.vtu; S

# External window test using GLFW
find_package(glfw3 QUIET)
if(glfw3_FOUND AND VTK_VERSION VERSION_GREATER 9.0.1)
  list(APPEND libf3dSDKTests_list
       TestSDKExternalWindowGLFW.cxx
      )
endif()

# External window test using QT
find_package(Qt5 QUIET COMPONENTS OpenGL)
if(Qt5_FOUND AND VTK_VERSION VERSION_GREATER 9.0.1)
  list(APPEND libf3dSDKTests_list
       TestSDKExternalWindowQT.cxx
      )
endif()

# create the testing file and list of tests
create_test_sourcelist(_libf3dSDKTests libf3dSDKTests.cxx ${libf3dSDKTests_list})

# add the executable
add_executable(libf3dSDKTests ${_libf3dSDKTests})
set_target_properties(libf3dSDKTests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  CXX_VISIBILITY_PRESET hidden
  )

# Add all the ADD_TEST for each test
foreach (test ${libf3dSDKTests_list})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME libf3d::${TName} COMMAND libf3dSDKTests ${TName}  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
endforeach ()

# Specific test properties to check

# CTest cant check colored output, info is not colored
set_tests_properties(libf3d::TestSDKLog PROPERTIES PASS_REGULAR_EXPRESSION "Test Info\nTest Warning\nTest Error\nTest Info Coloring")

target_link_libraries(libf3dSDKTests libf3d)

if(glfw3_FOUND)
  # external window test using glfw
  target_link_libraries(libf3dSDKTests glfw)
endif()

if(Qt5_FOUND)
  # external window test using QT
  target_link_libraries(libf3dSDKTests Qt5::OpenGL)
endif()

if (UNIX AND NOT APPLE)
  target_link_libraries(libf3dSDKTests stdc++fs)
endif ()

target_compile_features(libf3dSDKTests PRIVATE cxx_std_17)
