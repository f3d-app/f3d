cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-pdal)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(f3dPlugin)
endif()

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME LAS
  EXTENSIONS las laz
  MIMETYPES application/vnd.las
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "Lidar Data Exchange Format"
)

f3d_plugin_declare_reader(
  NAME E57
  EXTENSIONS e57
  MIMETYPES model/e57
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "ASTM E57 3D Data Exchange"
)

f3d_plugin_declare_reader(
  NAME PTS
  EXTENSIONS pts
  MIMETYPES application/vnd.pts
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "Laser scan plain data format"
)

f3d_plugin_declare_reader(
  NAME PTX
  EXTENSIONS ptx
  MIMETYPES application/vnd.ptx
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "ptx plain text data format"
)

f3d_plugin_declare_reader(
  NAME PCD
  EXTENSIONS pcd
  MIMETYPES application/vnd.pcd
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "point cloud data"
)

f3d_plugin_declare_reader(
  NAME EPT
  EXTENSIONS json
  MIMETYPES application/vnd.ept
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "Entwine Point Tile"
)

f3d_plugin_build(
  NAME pdal
  VERSION 1.0
  DESCRIPTION "VTK PDAL support"
  VTK_MODULES IOPDAL
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-pdal-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
