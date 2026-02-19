list(APPEND vtkextDraco_list
     TestF3DDracoReader.cxx
     TestF3DDracoReaderError.cxx
     TestF3DDracoReaderStream.cxx
     TestF3DDracoReaderStreamError.cxx
    )

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10884
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240214)
  list(APPEND vtkextDraco_list
    TestF3DGLTFDracoImporterStreamError.cxx
    )
endif()

vtk_add_test_cxx(vtkextDracoTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextDraco_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextDracoTests tests)

foreach(test ${vtkextDraco_list})
  get_filename_component (TName ${test} NAME_WE)
  set_tests_properties(f3d::vtkextDracoCxx-${TName} PROPERTIES
    LABELS "plugin;module;draco"
  )
endforeach()
