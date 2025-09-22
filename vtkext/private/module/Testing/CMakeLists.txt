set(test_sources
  TestF3DCachedTexturesPrint.cxx
  TestF3DGenericImporter.cxx
  TestF3DInteractorEventRecorder.cxx
  TestF3DLog.cxx
  TestF3DMetaImporterMultiColoring.cxx
  TestF3DMetaImporterAnimation.cxx
  TestF3DObjectFactory.cxx
  TestF3DOpenGLGridMapper.cxx
  TestF3DRenderPass.cxx
  TestF3DRendererWithColoring.cxx
  TestF3DFpsCounter.cxx
  )

if(F3D_MODULE_EXR)
  list(APPEND test_sources
       TestF3DEXRReader.cxx
       TestF3DEXRReaderInvalid.cxx
       TestF3DEXRMemReader.cxx)
endif()

if(F3D_MODULE_WEBP)
  list(APPEND test_sources
       TestF3DWebPReader.cxx
       TestF3DWebPReaderInvalid.cxx
       TestF3DWebPMemReader.cxx)
endif()

vtk_add_test_cxx(vtkextPrivateTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${test_sources}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextPrivateTests tests)
set_target_properties(vtkextPrivateTests PROPERTIES CXX_STANDARD 17)

if(UNIX)
  # On windows, the default window output message does not redirect to cout
  set_tests_properties(f3d::vtkextPrivateCxx-TestF3DLog PROPERTIES PASS_REGULAR_EXPRESSION "Test Info Test Warning Test Error\nTest Debug Test Info Test Warning Test Error\nTest Warning Test Error\nTest Error\nTest Info Coloring Test Warning Coloring Test Error Coloring\n")
endif()

set_tests_properties(f3d::vtkextPrivateCxx-TestF3DGenericImporter
  PROPERTIES
  FAIL_REGULAR_EXPRESSION "")

if(F3D_MODULE_EXR)
  set_tests_properties(f3d::vtkextPrivateCxx-TestF3DEXRReaderInvalid f3d::vtkextPrivateCxx-TestF3DEXRMemReader
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "")
endif()

if(F3D_MODULE_WEBP)
  set_tests_properties(f3d::vtkextPrivateCxx-TestF3DWebPReaderInvalid f3d::vtkextPrivateCxx-TestF3DWebPMemReader
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "")
endif()

if (F3D_MODULE_UI)
  target_compile_definitions(vtkextPrivateTests PRIVATE F3D_MODULE_UI)
endif ()
