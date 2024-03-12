# Find VTK at top level as f3d_vtkext-targets need it
find_package(VTK 9.0 REQUIRED COMPONENTS CommonCore CommonExecutionModel IOImport)
include("${CMAKE_CURRENT_LIST_DIR}/f3dPlugin.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../f3d_vtkext/f3d_vtkext-targets.cmake")

# Provide f3d_INCLUDE_DIR
set_and_check(f3d_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")

# Set the required variable
message(STATUS "Found f3d plugin component")
set(f3d_plugin_FOUND TRUE) 
