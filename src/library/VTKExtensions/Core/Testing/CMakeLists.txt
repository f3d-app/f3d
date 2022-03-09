# TODO add actual tests
vtk_add_test_cxx(VTKExtensionsCoreTests tests
  NO_DATA NO_VALID NO_OUTPUT
  TestF3DLog.cxx
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsCoreTests tests)
