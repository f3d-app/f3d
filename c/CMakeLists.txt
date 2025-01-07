cmake_minimum_required(VERSION 3.21)

project(f3d_c_api)

include(GNUInstallDirs)

# List of source files
set(F3D_C_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/image_c_api.cxx
  )

# List of headers that will be installed
set(F3D_C_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/image_c_api.h
  )

add_library(f3d_c_api ${F3D_C_SOURCE_FILES})

target_link_libraries(f3d_c_api PUBLIC libf3d)

target_include_directories(f3d_c_api
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )

set_target_properties(f3d_c_api PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  CXX_STANDARD 17
  POSITION_INDEPENDENT_CODE ON
  )

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# installing
install(TARGETS f3d_c_api
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT sdk EXCLUDE_FROM_ALL
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT sdk EXCLUDE_FROM_ALL
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT sdk EXCLUDE_FROM_ALL
  )

install(FILES ${F3D_C_PUBLIC_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/f3d"
  COMPONENT sdk
  EXCLUDE_FROM_ALL)
