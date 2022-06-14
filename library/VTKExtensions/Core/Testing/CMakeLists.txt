if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(VTKExtensionsCoreTests tests
  NO_DATA NO_VALID NO_OUTPUT
  TestF3DLog.cxx
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsCoreTests tests)

if(UNIX)
  # On windows, the default window output message does not redirect to cout
  set_tests_properties(f3d::VTKExtensionsCoreCxx-TestF3DLog PROPERTIES PASS_REGULAR_EXPRESSION "Test Info Test Warning Test Error\nTest Debug Test Info Test Warning Test Error\nTest Warning Test Error\nTest Error\nTest Info Coloring Test Warning Coloring Test Error Coloring\n")
endif()
