if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(VTKExtensionsRenderingTests tests
  NO_DATA NO_VALID NO_OUTPUT
  TestF3DInteractorEventRecorder.cxx
  TestF3DOpenGLGridMapper.cxx 
  TestF3DRenderPass.cxx
  TestF3DRenderer.cxx
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsRenderingTests tests)
