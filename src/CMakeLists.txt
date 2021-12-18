# TODO subdir
# Generate F3DIcon.h
add_executable(embed tools/embed.cxx)
target_link_libraries(embed PUBLIC VTK::IOImage)

# On Windows, add VTK IOImage library to PATH
# It is required to run `embed` with `add_custom_command`
# Other systems supports RPATH so it is not required
if(WIN32)
  get_target_property(_confs VTK::IOImage IMPORTED_CONFIGURATIONS)
  foreach(_conf ${_confs})
    get_target_property(_runtime_path VTK::IOImage IMPORTED_LOCATION_${_conf})
    get_filename_component(_runtime_dir ${_runtime_path} DIRECTORY)
    list(APPEND CMAKE_MSVCIDE_RUN_PATH "${_runtime_dir}")
  endforeach()
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.h
  COMMAND $<TARGET_FILE:embed>
    "${CMAKE_CURRENT_SOURCE_DIR}/../resources/logo32.png"
    "${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.h"
  DEPENDS embed)

cmake_dependent_option(F3D_MACOS_BUNDLE "Build a macOS bundle application" ON "APPLE" OFF)
cmake_dependent_option(F3D_WINDOWS_GUI "Build a non-console Win32 application" ON "WIN32" OFF)

set(LIB_PLATFORM_SPECIFIC_FILES "")
set(EXEC_PLATFORM_SPECIFIC_FILES "")
if(WIN32)
  set(EXEC_PLATFORM_SPECIFIC_FILES "${CMAKE_SOURCE_DIR}/resources/f3d.rc")
  if(F3D_WINDOWS_GUI)
    set(LIB_PLATFORM_SPECIFIC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/Misc/vtkF3DWin32OutputWindow.cxx")
  endif()
endif()
if(APPLE)
  set(LIB_PLATFORM_SPECIFIC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm")
endif()

set(MODULES_SPECIFIC_SOURCE_FILES "")
set(MODULES_SPECIFIC_INCLUDE_DIRS "")
set(MODULES_SPECIFIC_LIBRARIES "")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/core/F3DConfig.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/core/F3DConfig.h"
  @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/core/F3DConfig.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/core/F3DConfig.cxx"
  @ONLY)

# Readers declaration
add_subdirectory(readers)

if(F3D_MODULE_OCCT)
  set(MODULES_SPECIFIC_SOURCE_FILES ${MODULES_SPECIFIC_SOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/IO/vtkF3DOCCTReader.cxx")
  set(MODULES_SPECIFIC_INCLUDE_DIRS ${MODULES_SPECIFIC_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR})
  # Note: OpenCASCADE does not declare the libs in the correct dependency order.
  # This could be an issue on Linux when linking with static libraries.
  # See .github/workflows/ci.yml for a scripted fix 
  set(MODULES_SPECIFIC_LIBRARIES ${MODULES_SPECIFIC_LIBRARIES} ${OpenCASCADE_LIBRARIES})
endif()

if(F3D_MODULE_ASSIMP)
  set(MODULES_SPECIFIC_SOURCE_FILES ${MODULES_SPECIFIC_SOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/IO/vtkF3DAssimpImporter.cxx")
  set(MODULES_SPECIFIC_LIBRARIES ${MODULES_SPECIFIC_LIBRARIES} assimp::assimp)

  # Note: Assimp CMake module file does not declare Debug target properties, just map it to Release properties
  get_target_property(F3D_ASSIMP_DEBUG_VAR assimp::assimp IMPORTED_IMPLIB_RELEASE)
  if(NOT F3D_ASSIMP_DEBUG_VAR)
    set_target_properties(assimp::assimp PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release)
  endif()
endif()

# libf3d

set(F3D_SOURCE_FILES
  VTKExtensions/IO/vtkF3DGenericImporter.cxx
  VTKExtensions/IO/vtkF3DMetaReader.cxx
  VTKExtensions/Interaction/vtkF3DInteractorEventRecorder.cxx
  VTKExtensions/Interaction/vtkF3DInteractorStyle.cxx
  VTKExtensions/Misc/vtkF3DConsoleOutputWindow.cxx
  VTKExtensions/Misc/vtkF3DObjectFactory.cxx
  VTKExtensions/Misc/vtkF3DPostProcessFilter.cxx
  VTKExtensions/Rendering/vtkF3DOpenGLGridMapper.cxx
  VTKExtensions/Rendering/vtkF3DPolyDataMapper.cxx
  VTKExtensions/Rendering/vtkF3DRenderPass.cxx
  VTKExtensions/Rendering/vtkF3DRenderer.cxx
  VTKExtensions/Rendering/vtkF3DRendererWithColoring.cxx
  core/F3DAnimationManager.cxx
  core/F3DLoader.cxx
  core/F3DLog.cxx
  core/F3DOffscreenRender.cxx
  core/F3DOptions.cxx
  readers/F3DReader.cxx
  readers/F3DReaderFactory.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/core/F3DConfig.cxx
  ${MODULES_SPECIFIC_SOURCE_FILES}
  )

# Force static library when creating a macOS bundle
set(F3D_LIBRARY_TYPE "")
if(F3D_MACOS_BUNDLE)
  set(F3D_LIBRARY_TYPE "STATIC")
endif()

add_library(libf3d ${F3D_LIBRARY_TYPE}
  ${F3D_SOURCE_FILES}
  ${LIB_PLATFORM_SPECIFIC_FILES}
  )

target_include_directories(libf3d PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/IO
  ${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/Interaction
  ${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/Misc
  ${CMAKE_CURRENT_SOURCE_DIR}/VTKExtensions/Rendering
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/misc
  ${CMAKE_CURRENT_SOURCE_DIR}/readers
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/readers
  ${MODULES_SPECIFIC_INCLUDE_DIRS}
  )

set_target_properties(libf3d PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  WINDOWS_EXPORT_ALL_SYMBOLS ON
  PREFIX ""
  )

vtk_module_autoinit(TARGETS libf3d MODULES ${F3D_VTK_MODULES})

target_link_libraries(libf3d PUBLIC ${F3D_VTK_MODULES} ${MODULES_SPECIFIC_LIBRARIES})

set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warnings and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)
if(F3D_STRICT_BUILD)
  if(MSVC)
    # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
    target_compile_options(libf3d PUBLIC /W4 /WX /wd4275)
  else()
    target_compile_options(libf3d PUBLIC -Wall -Wextra -Wshadow -Werror)
  endif()
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

# coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
if(F3D_COVERAGE)
  target_compile_options(libf3d PUBLIC -g -O0 --coverage)
  target_link_options(libf3d PUBLIC --coverage)
endif()

# f3d

add_executable(f3d exec/main.cxx ${EXEC_PLATFORM_SPECIFIC_FILES})
target_link_libraries(f3d PUBLIC libf3d)
set_target_properties(f3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if (APPLE)
  set_target_properties(f3d PROPERTIES INSTALL_RPATH @loader_path/../lib)
elseif (UNIX)
  set_target_properties(f3d PROPERTIES INSTALL_RPATH $ORIGIN/../lib)
endif ()

if(F3D_STRICT_BUILD)
  if(MSVC)
    # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
    target_compile_options(f3d PUBLIC /W4 /WX /wd4275)
  else()
    target_compile_options(f3d PUBLIC -Wall -Wextra -Wshadow -Werror)
  endif()
endif()

# MacOS Bundle app
if(F3D_MACOS_BUNDLE)
  set_target_properties(f3d PROPERTIES MACOSX_BUNDLE TRUE)

  # Add default configuration
  if(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
    set(f3d_CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json)
    set_source_files_properties(${f3d_CONFIG} PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(f3d PRIVATE ${f3d_CONFIG})
  endif()

  # Add logo icon
  set(MACOSX_BUNDLE_ICON_FILE logo.icns)
  set(f3d_ICON ${CMAKE_SOURCE_DIR}/resources/logo.icns)
  set_source_files_properties(${f3d_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")
  target_sources(f3d PRIVATE ${f3d_ICON})
  configure_file("${CMAKE_SOURCE_DIR}/resources/BundleInfo.plist.in"
    "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
  set_target_properties(f3d PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/BundleInfo.plist")
endif()

# Windows executable without console
if(WIN32)
  if(F3D_WINDOWS_GUI)
    set_target_properties(f3d PROPERTIES WIN32_EXECUTABLE TRUE)
  else()
    # force usage of wWinMainCRTStartup in console mode for Unicode support
    target_link_options(f3d PUBLIC "/ENTRY:wWinMainCRTStartup")
  endif()
endif()

if(BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
  add_subdirectory(winshellext)
endif()
