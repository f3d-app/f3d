set(vtkextNativeTests_list "")

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12087
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  list(APPEND vtkextNativeTests_list
    TestF3DSPZReader.cxx
  )
endif()

vtk_add_test_cxx(vtkextNativeTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextNativeTests_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextNativeTests tests)
