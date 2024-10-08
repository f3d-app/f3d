# Find VTK and other dependencies at top level as f3d_vtkext-targets need it
if (UNIX AND NOT APPLE)
  find_package(X11)
endif()
find_package(OpenGL)
find_package(VTK 9.2.6 REQUIRED COMPONENTS CommonCore CommonExecutionModel IOImport)
include("${CMAKE_CURRENT_LIST_DIR}/f3dPlugin.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../f3d_vtkext/f3d_vtkext-targets.cmake")

# Provide f3d_INCLUDE_DIR
set_and_check(f3d_INCLUDE_DIR "${f3d_PREFIX_DIR}/include")

# Set the required variable
message(STATUS "Found f3d plugin component")
set(f3d_pluginsdk_FOUND TRUE) 
