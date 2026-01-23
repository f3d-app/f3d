# Find the web-ifc library
# Defines: web-ifc::web-ifc imported target

find_path(web-ifc_INCLUDE_DIR
  NAMES web-ifc/modelmanager/ModelManager.h
  PATH_SUFFIXES include
)

find_library(web-ifc_LIBRARY
  NAMES web-ifc-library web-ifc
  PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(web-ifc
  REQUIRED_VARS web-ifc_LIBRARY web-ifc_INCLUDE_DIR
)

if(web-ifc_FOUND AND NOT TARGET web-ifc::web-ifc)
  add_library(web-ifc::web-ifc UNKNOWN IMPORTED)
  set_target_properties(web-ifc::web-ifc PROPERTIES
    IMPORTED_LOCATION "${web-ifc_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${web-ifc_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(web-ifc_INCLUDE_DIR web-ifc_LIBRARY)
