if (NOT TARGET f3d::libf3d)
  include("${CMAKE_CURRENT_LIST_DIR}/library-config.cmake")
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/f3dCApiTargets.cmake")

set_and_check(f3d_C_API_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")

message(STATUS "Found f3d c_api component")
set(f3d_c_api_FOUND TRUE)
