set(test_sources
  TestF3DObjectFactory.cxx
  )

if(WIN32 AND F3D_WINDOWS_GUI)
  list(APPEND test_sources TestF3DWinGuiObjectFactory.cxx)
endif()

if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(VTKExtensionsApplicativeTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${test_sources}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsApplicativeTests tests)
