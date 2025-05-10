list(APPEND vtkextNativeTests_list
     TestF3DSPZReader.cxx
    )

vtk_add_test_cxx(vtkextNativeTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextNativeTests_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextNativeTests tests)
