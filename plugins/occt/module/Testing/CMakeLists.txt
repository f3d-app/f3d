list(APPEND vtkextOCCT_list
     TestF3DOCCTReader.cxx
    )

if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  set(CMAKE_WARN_DEPRECATED OFF)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(vtkextOCCTTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextOCCT_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextOCCTTests tests)
