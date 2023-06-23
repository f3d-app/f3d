# Modified from VTKDetermineVersion.cmake from VTK project
# Copyright (c) Kitware, Inc.
# License BSD 3 Clause

# Used to determine the version for F3D source using "git describe", if git
# is found. On success sets following variables in caller's scope:
#   ${var_prefix}_VERSION
#   ${var_prefix}_MAJOR_VERSION
#   ${var_prefix}_MINOR_VERSION
#   ${var_prefix}_PATCH_VERSION
#   ${var_prefix}_PATCH_VERSION_EXTRA
#   ${var_prefix}_VERSION_FULL
#   ${var_prefix}_VERSION_IS_RELEASE is true, if patch-extra is empty.
#
# If git is not found, or git describe cannot be run successfully, then these
# variables are left unchanged and status message is printed.
#
# This expect ${var_prefix}_MAJOR_VERSION, ${var_prefix}_MINOR_VERSION, ${var_prefix}_PATCH_VERSION
# and ${var_prefix}_VERSION to be already set and will be used for checking when possible.
#
# Arguments are:
#   source_dir : Source directory
#   git_command : git executable
#   var_prefix : prefix for variables e.g. "f3d".
function(f3d_determine_version source_dir git_command var_prefix)
  if (EXISTS ${git_command} AND
      EXISTS ${source_dir}/.git)
    execute_process(
      COMMAND ${git_command} describe
      WORKING_DIRECTORY ${source_dir}
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE)
    if (NOT result EQUAL "0")
      # git describe failed (bad return code).
      set(output "")
    endif()
  else ()
    set(output "")
  endif()

  unset(tmp_VERSION)
  f3d_extract_version_components("${output}" tmp)
  if(DEFINED tmp_VERSION)
    if (NOT tmp_VERSION STREQUAL ${var_prefix}_VERSION)
      message(WARNING
        "Version from git (${tmp_VERSION}) disagrees with hard coded version (${${var_prefix}_VERSION}). "
        "Either update the git tags or main CMakeLists.txt. Discarding version from git.")
    else ()
      foreach(suffix VERSION MAJOR_VERSION MINOR_VERSION PATCH_VERSION
                     PATCH_VERSION_EXTRA VERSION_FULL VERSION_IS_RELEASE)
        set(${var_prefix}_${suffix} ${tmp_${suffix}} PARENT_SCOPE)
      endforeach()
    endif()
  else()
    message(STATUS
      "Could not use git to determine source version, using version ${${var_prefix}_VERSION}")
  endif()
endfunction()

# Extracts components from a version string. See f3d_determine_version() for usage.
function(f3d_extract_version_components version_string var_prefix)
  string(REGEX MATCH "^v(([0-9]+)\\.([0-9]+)\\.([0-9]+)-?(.*))$"
    version_matches "${version_string}")
  if(CMAKE_MATCH_0)
    # note, we don't use CMAKE_MATCH_0 for `full` since it contains the `v` prefix.
    set(full ${CMAKE_MATCH_1})
    set(major ${CMAKE_MATCH_2})
    set(minor ${CMAKE_MATCH_3})
    set(patch ${CMAKE_MATCH_4})
    set(patch_extra ${CMAKE_MATCH_5})

    set(${var_prefix}_VERSION "${major}.${minor}.${patch}" PARENT_SCOPE)
    set(${var_prefix}_MAJOR_VERSION ${major} PARENT_SCOPE)
    set(${var_prefix}_MINOR_VERSION ${minor} PARENT_SCOPE)
    set(${var_prefix}_PATCH_VERSION ${patch} PARENT_SCOPE)
    set(${var_prefix}_PATCH_VERSION_EXTRA ${patch_extra} PARENT_SCOPE)
    set(${var_prefix}_VERSION_FULL ${full} PARENT_SCOPE)
    if("${major}.${minor}.${patch}" VERSION_EQUAL "${full}")
      set(${var_prefix}_VERSION_IS_RELEASE TRUE PARENT_SCOPE)
    else()
      set(${var_prefix}_VERSION_IS_RELEASE FALSE PARENT_SCOPE)
    endif()
  endif()
endfunction()
