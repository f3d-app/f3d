list(APPEND vtkextDraco_list
     TestF3DDracoReader.cxx
     TestF3DDracoReaderError.cxx
     TestF3DDracoReaderStream.cxx
     TestF3DDracoReaderStreamError.cxx
     TestF3DGLTFDracoImporterStreamError.cxx
    )

vtk_add_test_cxx(vtkextDracoTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextDraco_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextDracoTests tests)
