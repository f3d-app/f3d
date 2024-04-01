set(test_sources
  TestF3DCachedTexturesPrint.cxx
  TestF3DGenericImporter.cxx
  TestF3DGenericImporterMultiColoring.cxx
  TestF3DInteractorEventRecorder.cxx
  TestF3DLog.cxx
  TestF3DObjectFactory.cxx
  TestF3DOpenGLGridMapper.cxx 
  TestF3DRenderPass.cxx
  TestF3DRendererWithColoring.cxx
  )

if(WIN32 AND F3D_WINDOWS_GUI)
  list(APPEND test_sources TestF3DWinGuiObjectFactory.cxx)
endif()

if(F3D_MODULE_EXR)
  list(APPEND test_sources 
       TestF3DEXRReader.cxx
       TestF3DEXRReaderInvalid.cxx)
endif()

if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  set(CMAKE_WARN_DEPRECATED OFF)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(vtkextPrivateTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${test_sources}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextPrivateTests tests)

if(UNIX)
  # On windows, the default window output message does not redirect to cout
  set_tests_properties(f3d::vtkextPrivateCxx-TestF3DLog PROPERTIES PASS_REGULAR_EXPRESSION "Test Info Test Warning Test Error\nTest Debug Test Info Test Warning Test Error\nTest Warning Test Error\nTest Error\nTest Info Coloring Test Warning Coloring Test Error Coloring\n")
endif()

if(F3D_MODULE_EXR)
  set_tests_properties(f3d::vtkextPrivateCxx-TestF3DEXRReaderInvalid
    PROPERTIES 
    FAIL_REGULAR_EXPRESSION "")
endif()
