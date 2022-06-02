# TODO add actual tests
vtk_add_test_cxx(VTKExtensionsRenderingTests tests
  NO_DATA NO_VALID NO_OUTPUT
  TestF3DInteractorEventRecorder.cxx
  TestF3DOpenGLGridMapper.cxx 
  TestF3DRenderPass.cxx
  TestF3DRenderer.cxx
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsRenderingTests tests)
