# Override deb packaging installation location to /usr/local/
if (CPACK_GENERATOR STREQUAL "DEB")
  set(CPACK_PACKAGING_INSTALL_PREFIX /usr/local/)
endif()
