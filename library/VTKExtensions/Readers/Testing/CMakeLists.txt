list(APPEND VTKExtensionsReaderTests_list
     TestF3DMetaReader.cxx
     TestF3DMetaReaderInvalid.cxx
     TestF3DGenericImporter.cxx
    )

if(F3D_MODULE_ALEMBIC)
  list(APPEND VTKExtensionsReaderTests_list TestF3DAlembicImporter.cxx)
endif()

if(F3D_MODULE_ASSIMP)
  list(APPEND VTKExtensionsReaderTests_list TestF3DAssimpImporter.cxx)
endif()

if(F3D_MODULE_OCCT)
  list(APPEND VTKExtensionsReaderTests_list TestF3DOCCTReader.cxx)
endif()

cmake_policy(SET CMP0115 NEW)
vtk_add_test_cxx(VTKExtensionsReaderTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsReaderTests_list}
  ${CMAKE_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsReaderTests tests)

set_tests_properties(f3d::VTKExtensionsReadersCxx-TestF3DMetaReaderInvalid PROPERTIES WILL_FAIL TRUE)
