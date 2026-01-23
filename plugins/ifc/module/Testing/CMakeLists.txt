list(APPEND VTKExtensionsPluginIFC_list
     TestF3DIFCReader.cxx
     TestF3DIFCReaderOptions.cxx
     TestF3DIFCReaderError.cxx
    )

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  list(APPEND VTKExtensionsPluginIFC_list
       TestF3DIFCReaderStream.cxx
       TestF3DIFCReaderStreamError.cxx
      )
endif()

vtk_add_test_cxx(VTKExtensionsPluginIFC tests
  NO_DATA NO_VALID NO_OUTPUT
  ${VTKExtensionsPluginIFC_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(VTKExtensionsPluginIFC tests)
