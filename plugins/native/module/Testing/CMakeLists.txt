set(vtkextNativeTests_list TestF3DImageImporterError.cxx)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12087
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  list(APPEND vtkextNativeTests_list
    TestF3DSPZReader.cxx
    TestF3DSplatReader.cxx
    TestF3DPLYReader.cxx
    TestF3DQuakeMDLImporterStream.cxx
    TestF3DQuakeMDLImporterStreamError.cxx
    TestF3DQuakeMDLImporterInexistent.cxx
    TestF3DQuakeMDLParser.cxx
  )
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260128)
  list(APPEND vtkextNativeTests_list
    TestF3DImageImporterCanReadFile.cxx)
endif()

vtk_add_test_cxx(vtkextNativeTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextNativeTests_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextNativeTests tests)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  set_tests_properties(f3d::vtkextNativeCxx-TestF3DPLYReader
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "")
  set_tests_properties(f3d::vtkextNativeCxx-TestF3DQuakeMDLImporterInexistent
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "")
endif()
