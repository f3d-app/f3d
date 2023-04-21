list(APPEND VTKExtensionsPluginAssimp_list
     TestF3DAssimpImporter.cxx
     TestF3DAssimpImportError.cxx
    )

if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(VTKExtensionsPluginAssimp tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginAssimp_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsPluginAssimp tests)
