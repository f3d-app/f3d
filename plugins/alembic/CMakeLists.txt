include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

find_package(Alembic 1.7 REQUIRED)

message(STATUS "Plugin: Alembic ${Alembic_VERSION} found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME Alembic
  SCORE 30
  EXTENSIONS abc
  MIMETYPES application/vnd.abc
  VTK_IMPORTER vtkF3DAlembicImporter
  DESCRIPTION "Alembic files reader"
)

f3d_plugin_build(
  NAME alembic
  VERSION 1.0
  DESCRIPTION "Alembic support (version ${Alembic_VERSION})"
  VTK_MODULES CommonCore CommonExecutionModel IOImport
)
