include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME ExodusII
  EXTENSIONS exo ex2 e
  MIMETYPES application/vnd.exodus
  VTK_READER vtkExodusIIReader
  DESCRIPTION "ExodusII files reader"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/exodus.inl"
)

f3d_plugin_build(
  NAME exodus
  VERSION 1.0
  DESCRIPTION "VTK Exodus support"
  VTK_MODULES CommonCore CommonExecutionModel IOImport IOExodus
)
