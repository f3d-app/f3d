cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-open3sdcm LANGUAGES CXX)

include(GNUInstallDirs)

# Only internal build is supported for static plugins
if(PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "open3sdcm plugin cannot be built externally")
endif()

include(f3dPlugin)

# Open3SDCM library
# For development, use local path. For production, consider using FetchContent or git submodule
option(OPEN3SDCM_USE_LOCAL "Use local Open3SDCM path for development" OFF)
if(OPEN3SDCM_USE_LOCAL)
  # Local development path
  set(OPEN3SDCM_SOURCE_DIR "/Users/romainnosenzo/CLionProjects/Open3SDCM")
  if(NOT EXISTS "${OPEN3SDCM_SOURCE_DIR}/Lib/CMakeLists.txt")
    message(FATAL_ERROR "Open3SDCM not found at ${OPEN3SDCM_SOURCE_DIR}")
  endif()
  
  # Add Open3SDCM as subdirectory
  add_subdirectory(${OPEN3SDCM_SOURCE_DIR}/Lib external/Open3SDCM)
  
  # Remove unused Boost dependency from Open3SDCM (not actually used in ParseDcm)
  # We'll patch it by removing the boost_dynamic_bitset requirement
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_MULTITHREADED ON)
else()
  # For CI/production, use FetchContent
  include(FetchContent)
  FetchContent_Declare(
    Open3SDCM
    GIT_REPOSITORY https://github.com/Nosenzor/Open3SDCM.git
    GIT_TAG master
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(Open3SDCM)
endif()

# Required dependencies for Open3SDCM
# Poco is required for XML parsing and ZIP extraction
find_package(Poco CONFIG REQUIRED COMPONENTS XML JSON Zip)

# OpenSSL is required for CE schema (encrypted files)
find_package(OpenSSL REQUIRED)

# Boost is required by Open3SDCM
find_package(Boost REQUIRED)

# Initialize plugin
f3d_plugin_init()

# Declare the reader/importer for 3Shape DCM files
# Note: .dcm extension is also used by DICOM reader
# We use SCORE 70 (> DICOM's 50) + content-based canRead() to resolve conflict
f3d_plugin_declare_reader(
  NAME Open3SDCM
  EXTENSIONS dcm
  MIMETYPES application/vnd.3shape-dcm
  VTK_IMPORTER vtkF3DOpen3SDCMImporter
  FORMAT_DESCRIPTION "3Shape DCM (Dental CAD Model)"
  SCORE 70
  CAN_READ STATIC
)

# Add Open3SDCM include directory for the generated reader code
list(APPEND F3D_PLUGIN_INCLUDES ${Open3SDCM_SOURCE_DIR}/Lib/src)

# Build the plugin
f3d_plugin_build(
  NAME open3sdcm
  VERSION 1.0
  DESCRIPTION "3Shape DCM file format support via Open3SDCM library"
  VTK_MODULES IOGeometry IOImage RenderingCore
  FREEDESKTOP
)

# Link Open3SDCM dependencies to our VTK reader
# This is done automatically by f3d_plugin_build, but we need to ensure
# the Open3SDCM library is linked properly

# Link the plugin to the VTK reader library and Open3SDCM
# This needs to be done after f3d_plugin_build creates the target
target_link_libraries(f3d-plugin-open3sdcm PRIVATE
  f3d::vtkextOpen3SDCM
  Open3SDCMLib
  Poco::XML
  Poco::Zip
  OpenSSL::Crypto
  ${Boost_LIBRARIES}
)

# Add Open3SDCM include directory to the plugin target
# This is needed for the custom code (.inl file) which includes ParseDcm.h
target_include_directories(f3d-plugin-open3sdcm
  PRIVATE
    /Users/romainnosenzo/CLionProjects/Open3SDCM/Lib/src
)

# Print configuration summary
message(STATUS "Plugin: open3sdcm configured")
message(STATUS "  Open3SDCM source: ${OPEN3SDCM_SOURCE_DIR}")
message(STATUS "  Poco found: ${Poco_DIR}")
message(STATUS "  OpenSSL found: ${OPENSSL_DIR}")

message(STATUS "  Boost found: ${Boost_DIR}")
