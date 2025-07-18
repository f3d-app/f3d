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
    NAME PDAL
    EXTENSIONS arrow bpf ilvis2 e57 fbi mbio npy csd pcd ply qfit rdb rxp sbet bin tiledb las drc ept.json json hdf h5 mat ntif obj pts ptx txt splk spz
    MIMETYPES application/octet-stream application/vnd.las model/draco application/json application/x-hdf application/x-matlab application/vnd.nitf text/plain application/vnd.splk application/vnd.spz
    FORMAT_DESCRIPTION "pdal processes a variety of files: https://pdal.io/en/stable/stages/readers.html"
    CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/pdal.inl"
)

f3d_plugin_build(
  NAME pdal
  VERSION 1.0
  DESCRIPTION "VTK IOPDAL support"
  VTK_MODULES IOPDAL
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-pdal-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)