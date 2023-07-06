list(APPEND VTKExtensionsPluginDraco_list
     TestF3DDracoReader.cxx
    )

if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(VTKExtensionsPluginDraco tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginDraco_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsPluginDraco tests)
