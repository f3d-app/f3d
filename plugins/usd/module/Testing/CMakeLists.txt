list(APPEND VTKExtensionsPluginUSD_list
     TestF3DUSDImporter.cxx
    )

vtk_add_test_cxx(VTKExtensionsPluginUSD tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginUSD_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsPluginUSD tests)
