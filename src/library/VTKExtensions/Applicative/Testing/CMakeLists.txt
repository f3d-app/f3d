# TODO add actual test
vtk_add_test_cxx(VTKExtensionsApplicativeTests tests
  NO_DATA NO_VALID NO_OUTPUT
  TestF3DObjectFactory.cxx
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsApplicativeTests tests)
