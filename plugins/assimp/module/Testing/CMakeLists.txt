set(VTKExtensionsPluginAssimp_list)

# Sanitizer exclusion because of https://github.com/f3d-app/f3d/issues/1323
if(NOT F3D_SANITIZER STREQUAL "address")
  list(APPEND VTKExtensionsPluginAssimp_list
       TestF3DAssimpImporter.cxx
       TestF3DAssimpImportError.cxx
       TestF3DAssimpImporterTimeSteps.cxx
      )

  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251016)
    list(APPEND VTKExtensionsPluginAssimp_list
         TestF3DAssimpImporterStream.cxx
         TestF3DAssimpImporterStreamError.cxx
        )
  endif()
endif()

vtk_add_test_cxx(VTKExtensionsPluginAssimp tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginAssimp_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsPluginAssimp tests)
