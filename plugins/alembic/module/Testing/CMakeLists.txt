list(APPEND VTKExtensionsPluginAlembic_list
     TestF3DAlembicReader.cxx
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  list(APPEND VTKExtensionsPluginAlembic_list
       TestF3DAlembicReaderStream.cxx
       TestF3DAlembicReaderStreamError.cxx
      )
endif()

vtk_add_test_cxx(vtkextAlembicTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginAlembic_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextAlembicTests tests)

foreach(test ${VTKExtensionsPluginAlembic_list})
  get_filename_component (TName ${test} NAME_WE)
  set_tests_properties(f3d::vtkextAlembicCxx-${TName} PROPERTIES
    LABELS "module;plugins;alembic;abc"
  )
endforeach()
