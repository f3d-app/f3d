# Provide f3d::f3d target
include("${CMAKE_CURRENT_LIST_DIR}/f3dTargets.cmake")

# Provide a f3d_embed_file method for embedding image into source code
include("${CMAKE_CURRENT_LIST_DIR}/f3dEmbed.cmake")

# Set the required variable
message(STATUS "Found f3d application component")
set(f3d_application_FOUND TRUE) 
