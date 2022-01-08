# Generate F3DIcon buffer
vtk_encode_string(
  INPUT "${CMAKE_SOURCE_DIR}/resources/logo32.png"
  NAME F3DIcon
  HEADER_OUTPUT F3D_ICON_HEADER
  SOURCE_OUTPUT F3D_ICON_SOURCE
  BINARY
  NUL_TERMINATE)

# Define libf3d target
set(LIB_PLATFORM_SPECIFIC_FILES "")
if(WIN32 AND F3D_WINDOWS_GUI)
  set(LIB_PLATFORM_SPECIFIC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DWin32OutputWindow.cxx")
endif()
if(APPLE)
  set(LIB_PLATFORM_SPECIFIC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm")
endif()

set(MODULES_SPECIFIC_SOURCE_FILES "")
set(MODULES_SPECIFIC_INCLUDE_DIRS "")
set(MODULES_SPECIFIC_LIBRARIES "")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.h"
  @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.cxx"
  @ONLY)

# Readers declaration
add_subdirectory(readers)

if(F3D_MODULE_OCCT)
  set(MODULES_SPECIFIC_SOURCE_FILES ${MODULES_SPECIFIC_SOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DOCCTReader.cxx")
  set(MODULES_SPECIFIC_INCLUDE_DIRS ${MODULES_SPECIFIC_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR})
  # Note: OpenCASCADE does not declare the libs in the correct dependency order.
  # This could be an issue on Linux when linking with static libraries.
  # See .github/workflows/ci.yml for a scripted fix 
  set(MODULES_SPECIFIC_LIBRARIES ${MODULES_SPECIFIC_LIBRARIES} ${OpenCASCADE_LIBRARIES})
endif()

if(F3D_MODULE_ASSIMP)
  set(MODULES_SPECIFIC_SOURCE_FILES ${MODULES_SPECIFIC_SOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DAssimpImporter.cxx")
  set(MODULES_SPECIFIC_LIBRARIES ${MODULES_SPECIFIC_LIBRARIES} assimp::assimp)

  # Note: Assimp CMake module file does not declare Debug target properties, just map it to Release properties
  get_target_property(F3D_ASSIMP_DEBUG_VAR assimp::assimp IMPORTED_IMPLIB_RELEASE)
  if(NOT F3D_ASSIMP_DEBUG_VAR)
    set_target_properties(assimp::assimp PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release)
  endif()
endif()

if(F3D_MODULE_ALEMBIC)
  set(MODULES_SPECIFIC_SOURCE_FILES ${MODULES_SPECIFIC_SOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DAlembicImporter.cxx")
  set(MODULES_SPECIFIC_LIBRARIES ${MODULES_SPECIFIC_LIBRARIES} Alembic::Alembic)
endif()

set(F3D_SOURCE_FILES
  vtkF3DConsoleOutputWindow.cxx
  vtkF3DGenericImporter.cxx
  vtkF3DInteractorEventRecorder.cxx
  vtkF3DInteractorStyle.cxx
  vtkF3DMetaReader.cxx
  vtkF3DObjectFactory.cxx
  vtkF3DOpenGLGridMapper.cxx
  vtkF3DPolyDataMapper.cxx
  vtkF3DPostProcessFilter.cxx
  vtkF3DRenderPass.cxx
  vtkF3DRenderer.cxx
  vtkF3DRendererWithColoring.cxx
  F3DAnimationManager.cxx
  F3DLoader.cxx
  F3DLog.cxx
  F3DOffscreenRender.cxx
  F3DOptions.cxx
  readers/F3DReader.cxx
  readers/F3DReaderFactory.cxx
  ${F3D_ICON_SOURCE}
  ${CMAKE_CURRENT_BINARY_DIR}/Config.cxx
  ${MODULES_SPECIFIC_SOURCE_FILES}
  )

set(F3D_HEADER_FILES
  vtkF3DConsoleOutputWindow.h
  vtkF3DGenericImporter.h
  vtkF3DInteractorEventRecorder.h
  vtkF3DInteractorStyle.h
  vtkF3DMetaReader.h
  vtkF3DObjectFactory.h
  vtkF3DOpenGLGridMapper.h
  vtkF3DPolyDataMapper.h
  vtkF3DPostProcessFilter.h
  vtkF3DRenderPass.h
  vtkF3DRenderer.h
  vtkF3DRendererWithColoring.h
  F3DAnimationManager.h
  F3DLoader.h
  F3DLog.h
  readers/F3DReader.h
  readers/F3DReaderFactory.h
  ${F3D_ICON_HEADER}
  ${CMAKE_CURRENT_BINARY_DIR}/readers/F3DReaderInstantiator.h
  )

# Force static library when creating a macOS bundle
set(F3D_LIBRARY_TYPE "")
if(F3D_MACOS_BUNDLE)
  set(F3D_LIBRARY_TYPE "STATIC")
endif()

add_library(libf3d ${F3D_LIBRARY_TYPE}
  ${F3D_SOURCE_FILES}
  ${F3D_HEADER_FILES}
  ${LIB_PLATFORM_SPECIFIC_FILES}
  )

target_include_directories(libf3d PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/readers
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

if(F3D_STRICT_BUILD)
  if(MSVC)
    # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
    target_compile_options(libf3d PUBLIC /W4 /WX /wd4275)
  else()
    target_compile_options(libf3d PUBLIC -Wall -Wextra -Wshadow -Werror)
  endif()
endif()

# coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
if(F3D_COVERAGE)
  target_compile_options(libf3d PUBLIC -g -O0 --coverage)
  target_link_options(libf3d PUBLIC --coverage)
endif()
