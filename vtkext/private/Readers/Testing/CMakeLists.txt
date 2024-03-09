list(APPEND VTKExtensionsReaderTests_list
     TestF3DGenericImporter.cxx
     TestF3DGenericImporterMultiColoring.cxx
    )

if(F3D_MODULE_EXR)
  list(APPEND VTKExtensionsReaderTests_list 
       TestF3DEXRReader.cxx
       TestF3DEXRReaderInvalid.cxx)
endif()

if(VTK_VERSION VERSION_LESS_EQUAL 9.1.0)
  set(CMAKE_WARN_DEPRECATED OFF)
  cmake_policy(SET CMP0115 OLD)
endif()

vtk_add_test_cxx(VTKExtensionsReaderTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsReaderTests_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsReaderTests tests)

if(F3D_MODULE_EXR)
set_tests_properties(f3d::VTKExtensionsReadersCxx-TestF3DEXRReaderInvalid
  PROPERTIES 
    FAIL_REGULAR_EXPRESSION "")
endif()
