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
  MIMETYPES application/vnd.las application/vnd.laszip
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "LASer format for lidar point cloud"
)

f3d_plugin_declare_reader(
  NAME BinaryPointFile
  EXTENSIONS bpf
  MIMETYPES application/vnd.bfp3
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Binary Point File"
)

f3d_plugin_declare_reader(
  NAME TerraScanBIN
  EXTENSIONS bin
  MIMETYPES application/vnd.terrascan.binary
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "TerraScan binary files"
)

f3d_plugin_declare_reader(
  NAME OptechCSD
  EXTENSIONS csd
  MIMETYPES application/vnd.csd
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Optech Corrected Sensor Data"
)

f3d_plugin_declare_reader(
  NAME PCD
  EXTENSIONS pcd
  MIMETYPES application/vnd.pcd
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Point Cloud Data"
)

f3d_plugin_declare_reader(
  NAME LeicaCyclonePTX
  EXTENSIONS ptx
  MIMETYPES application/vnd.ptx
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Leica Cyclone"
)

f3d_plugin_declare_reader(
  NAME QFIT
  EXTENSIONS qi
  MIMETYPES application/vnd.qi
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Airborne Topographic Mapper QFIT"
)

f3d_plugin_declare_reader(
  NAME SBET
  EXTENSIONS sbet
  MIMETYPES application/vnd.sbet
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Smoothed Best Estimate of Trajectory"
)

f3d_plugin_declare_reader(
  NAME SLPK
  EXTENSIONS slpk
  MIMETYPES application/vnd.slpk
  VTK_READER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Esri Scene Layer Package"
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

# PDAL supports static build
# github.com/PDAL/PDAL/pull/4996
# but require symbols to be specifically included on link
# please note this is only tested in f3d-superbuild
if (TARGET pdalcpp)
  get_target_property(_target_type pdalcpp TYPE)
  if (_target_type STREQUAL STATIC_LIBRARY)
    if (WIN32)
      foreach (_pdal_driver Qfit Sbet Slpk Optech Terrasolid Bpf Las Pcd Ptx)
        target_link_options(pdalcpp INTERFACE "/INCLUDE:??0${_pdal_driver}Reader@pdal@@QEAA@XZ")
      endforeach ()
    else ()
      foreach (_pdal_driver 10QfitReaderC1Ev 10SbetReaderC1Ev 10SlpkReaderC1Ev 12OptechReaderC1Ev 16TerrasolidReaderD0Ev 9BpfReaderC1Ev 9LasReaderC1Ev 9PcdReaderC1Ev 9PtxReaderD0Ev)
        if (APPLE)
          target_link_options(pdalcpp INTERFACE "-Wl,-u,__ZN4pdal${_pdal_driver}")
        elseif (UNIX)
          target_link_options(pdalcpp INTERFACE "-Wl,--undefined=_ZN4pdal${_pdal_driver}")
        endif ()
      endforeach ()
    endif ()
  endif ()
endif ()
