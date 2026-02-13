set(VTKExtensionsPluginAssimp_list
    TestF3DAssimpImportCanReadFile.cxx)

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

vtk_add_test_cxx(vtkextAssimpTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginAssimp_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextAssimpTests tests)
set_target_properties(vtkextAssimpTests PROPERTIES CXX_STANDARD 17)

foreach(test ${VTKExtensionsPluginAssimp_list})
  get_filename_component (TName ${test} NAME_WE)
  set_tests_properties(f3d::vtkextAssimpCxx-${TName} PROPERTIES
    LABELS "plugins;module;assimp"
  )
endforeach()
