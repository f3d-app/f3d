cmake_minimum_required(VERSION 3.21)

project(f3d_c_api)

include(GNUInstallDirs)

# List of source files
set(F3D_C_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/camera_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/context_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/engine_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/image_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/interactor_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/log_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/options_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/scene_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/types_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/utils_c_api.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/window_c_api.cxx
)

# List of headers that will be installed
set(F3D_C_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/camera_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/context_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/engine_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/interactor_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/log_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/options_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/scene_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/types_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/utils_c_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/window_c_api.h
)

add_library(c_api ${F3D_C_SOURCE_FILES})
set_target_properties(c_api PROPERTIES
  OUTPUT_NAME "f3d_c_api"
  CXX_VISIBILITY_PRESET hidden
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(c_api PRIVATE libf3d)

target_include_directories(c_api
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${F3D_BINARY_DIR}/library/public>
  INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

# Required because we use libf3d own export.h
target_compile_definitions(c_api PRIVATE libf3d_EXPORTS)

target_compile_options(c_api PUBLIC ${f3d_sanitizer_compile_options})
target_link_options(c_api PUBLIC ${f3d_sanitizer_link_options})

if(APPLE)
  set_target_properties(c_api PROPERTIES INSTALL_RPATH "@loader_path")
elseif(UNIX)
  set_target_properties(c_api PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# installing
if (BUILD_SHARED_LIBS)

  # lib
  install(TARGETS c_api
    EXPORT f3dCApiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT c_api
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT c_api
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT c_api
  )

  # includes
  install(FILES ${F3D_C_PUBLIC_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/f3d"
    COMPONENT c_api)

  # cmake
  export(TARGETS c_api
    NAMESPACE f3d::
    APPEND
    FILE "${CMAKE_BINARY_DIR}/cmake/f3dCApiTargets.cmake")

  install(EXPORT f3dCApiTargets
    NAMESPACE f3d::
    FILE f3dCApiTargets.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
    COMPONENT sdk EXCLUDE_FROM_ALL
  )

  file(COPY "${F3D_SOURCE_DIR}/cmake/c_api-config.cmake"
    DESTINATION "${CMAKE_BINARY_DIR}/cmake")

  install(
    FILES
    "${F3D_SOURCE_DIR}/cmake/c_api-config.cmake"
    DESTINATION
    "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
    COMPONENT sdk EXCLUDE_FROM_ALL)
endif ()
