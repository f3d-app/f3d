# Provide f3d::libf3d target
include("${CMAKE_CURRENT_LIST_DIR}/f3dLibraryTargets.cmake")

# Provide f3d_INCLUDE_DIR
set_and_check(f3d_INCLUDE_DIR "${f3d_PREFIX_DIR}/include")

# Set the required variable
message(STATUS "Found f3d library component")
set(f3d_library_FOUND TRUE) 
