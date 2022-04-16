
cmake_dependent_option(F3D_MACOS_BUNDLE "Build a macOS bundle application" ON "APPLE" OFF)
cmake_dependent_option(F3D_WINDOWS_GUI "Build a non-console Win32 application" ON "WIN32" OFF)

# Force static library when creating a macOS bundle
cmake_dependent_option(BUILD_SHARED_LIBS "Build f3d with shared libraries" ON "NOT F3D_MACOS_BUNDLE" OFF)

set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warnings and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)

# libf3d target
add_subdirectory(library)

# f3d target
add_subdirectory(application)

if(BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
  add_subdirectory(winshellext)
endif()

# Create man
if(F3D_GENERATE_MAN)
  find_program(HELP2MAN help2man)
  find_program(GZIP gzip)
  if(HELP2MAN AND GZIP)
    set(MAN_OUTPUT_FILE ${CMAKE_BINARY_DIR}/f3d.1)
    add_custom_command(
      OUTPUT ${MAN_OUTPUT_FILE}
      COMMAND ${HELP2MAN} $<TARGET_FILE:f3d> -N -n "F3D - Fast and minimalist 3D viewer" > ${MAN_OUTPUT_FILE}
      COMMAND ${GZIP} -f ${MAN_OUTPUT_FILE}
      DEPENDS f3d)
    add_custom_target(man ALL DEPENDS ${MAN_OUTPUT_FILE})
  else()
    message(WARNING "help2man or gzip not found, cannot create man entry")
  endif()
endif()
