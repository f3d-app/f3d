if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  set(CMAKE_WARN_DEPRECATED OFF)
  cmake_policy(SET CMP0115 OLD)
endif()

list(APPEND VTKExtensionsRenderingTests_list
     TestF3DInteractorEventRecorder.cxx
     TestF3DOpenGLGridMapper.cxx 
     TestF3DRenderPass.cxx
     TestF3DRendererWithColoring.cxx
     TestF3DCachedTexturesPrint.cxx
  )

vtk_add_test_cxx(VTKExtensionsRenderingTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsRenderingTests_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsRenderingTests tests)
