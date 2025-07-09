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
    EXTENSIONS arrow bpf draco json E57 fbi hdf ilvis2 las mat npy obj csd pcd ply pts ptx qfit rdb rxp sbet slpk spz bin txt 
    MIMETYPES application/vnd.apache.arrow.file application/vnd.bpf application/octet-stream model/vnd.google.draco model/e57 application/x-hdf image/vnd-ms.dds text/plain application/x-matlab-data
    FORMAT_DESCRIPTION "pdal processes a variety of files: https://pdal.io/en/stable/stages/readers.html"
    CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/pdal.inl"
)

f3d_plugin_build(
  NAME pdal
  VERSION 1.0
  DESCRIPTION "VTK PDAL support"
)