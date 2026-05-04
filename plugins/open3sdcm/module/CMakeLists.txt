# Open3SDCM plugin module

# Include directory for this module
set(MODULE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(MODULE_SOURCES
  vtkF3DOpen3SDCMReader.cxx
)

# Create the VTK reader class
vtk_add_library(vtkF3DOpen3SDCMReader
  ${MODULE_SOURCES}
)

# Include directories
target_include_directories(vtkF3DOpen3SDCMReader
  PUBLIC
    ${MODULE_INCLUDE_DIR}
  PRIVATE
    ${Open3SDCM_SOURCE_DIR}/Lib/src
)

# Link against Open3SDCM library
target_link_libraries(vtkF3DOpen3SDCMReader PRIVATE Open3SDCMLib)

# Link against dependencies
target_link_libraries(vtkF3DOpen3SDCMReader PRIVATE
  Poco::XML
  Poco::Zip
  OpenSSL::Crypto
  assimp::assimp
)

# Set C++20 standard (F3D already uses C++20, Open3SDCM uses C++20 features)
target_compile_features(vtkF3DOpen3SDCMReader PRIVATE cxx_std_20)

# VTK module file
set(VTK_MODULE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vtk.module")

# Export for the plugin
set_property(TARGET vtkF3DOpen3SDCMReader PROPERTY EXPORT_NAME F3DOpen3SDCM)
