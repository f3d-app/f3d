list(APPEND vtkextOCCT_list
     TestF3DOCCTReader.cxx
     TestF3DOCCTReaderCanReadFile.cxx
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251223)
  list(APPEND vtkextOCCT_list
     TestF3DOCCTReaderStream.cxx
     TestF3DOCCTReaderStreamError.cxx
      )
endif()

vtk_add_test_cxx(vtkextOCCTTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextOCCT_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextOCCTTests tests)
set_target_properties(vtkextOCCTTests PROPERTIES CXX_STANDARD 17)

if(F3D_PLUGIN_OCCT_COLORING_SUPPORT)
  target_compile_definitions(vtkextOCCTTests
    PRIVATE F3D_PLUGIN_OCCT_XCAF)
endif()
