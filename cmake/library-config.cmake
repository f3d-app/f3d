# Provide f3d::f3d target
include("${CMAKE_CURRENT_LIST_DIR}/f3dLibraryTargets.cmake")

# Provide f3d_INCLUDE_DIR
set_and_check(f3d_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")

# Set the required variable
set(f3d_library_FOUND TRUE) 
