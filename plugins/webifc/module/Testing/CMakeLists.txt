list(APPEND vtkextWebIFCTests_list
     TestF3DWebIFCReader.cxx
     TestF3DWebIFCReaderAPI.cxx
     TestF3DWebIFCReaderError.cxx
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  list(APPEND vtkextWebIFCTests_list
       TestF3DWebIFCReaderStream.cxx
       TestF3DWebIFCReaderStreamError.cxx
      )
endif()

vtk_add_test_cxx(vtkextWebIFCTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextWebIFCTests_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextWebIFCTests tests)

foreach(test ${vtkextWebIFCTests_list})
  get_filename_component (TName ${test} NAME_WE)
  set_tests_properties(f3d::vtkextWebIFCCxx-${TName} PROPERTIES
    LABELS "plugin;module;webifc"
  )
endforeach()
