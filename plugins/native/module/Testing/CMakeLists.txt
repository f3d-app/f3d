set(vtkextNative_list
     TestF3DQuakeMDLImporter.cxx
    )

vtk_add_test_cxx(vtkextNativeTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextNative_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)

vtk_test_cxx_executable(vtkextNativeTests tests)