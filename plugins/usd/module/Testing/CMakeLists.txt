list(APPEND VTKExtensionsPluginUSD_list
     TestF3DUSDImporter.cxx
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251016)
  list(APPEND VTKExtensionsPluginUSD_list
        TestF3DUSDImporterStream.cxx
        TestF3DUSDImporterStreamError.cxx
      )
endif()

vtk_add_test_cxx(vtkextUSDTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginUSD_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextUSDTests tests)

foreach(test ${VTKExtensionsPluginUSD_list})
  get_filename_component (TName ${test} NAME_WE)
  set_tests_properties(f3d::vtkextUSDCxx-${TName} PROPERTIES
    LABELS "plugin;module;usd"
  )
endforeach()
