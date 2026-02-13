set(vtkextTests_list)

# Also needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
# Sanitizer exclusion because of https://github.com/f3d-app/f3d/issues/1323
if(NOT ANDROID AND NOT EMSCRIPTEN AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203 AND NOT F3D_SANITIZER STREQUAL "address")
  list(APPEND vtkextTests_list 
       TestF3DBitonicSort.cxx)
endif()

if(F3D_MODULE_EXR)
  list(APPEND vtkextTests_list
       TestF3DEXRReader.cxx
       TestF3DEXRReaderInvalid.cxx
       TestF3DEXRMemReader.cxx)
endif()

if(F3D_MODULE_WEBP)
  list(APPEND vtkextTests_list
       TestF3DWebPReader.cxx
       TestF3DWebPReaderInvalid.cxx
       TestF3DWebPMemReader.cxx)
endif()

vtk_add_test_cxx(vtkextTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextTests_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextTests tests)

if(F3D_MODULE_EXR)
  set_tests_properties(f3d::vtkextCxx-TestF3DEXRReaderInvalid f3d::vtkextCxx-TestF3DEXRMemReader
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "")
endif()

if(F3D_MODULE_WEBP)
  set_tests_properties(f3d::vtkextCxx-TestF3DWebPReaderInvalid f3d::vtkextCxx-TestF3DWebPMemReader
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "")
endif()
