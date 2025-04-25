list(APPEND VTKExtensionsPluginSPZ_list
     TestF3DSPZReader.cxx
    )

vtk_add_test_cxx(VTKExtensionsPluginSPZ tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginSPZ_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsPluginSPZ tests)
