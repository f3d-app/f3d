# Generate F3DIcon buffer
vtk_encode_string(
  INPUT "${CMAKE_SOURCE_DIR}/resources/logo32.png"
  NAME F3DIcon
  BINARY)

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
  "${CMAKE_CURRENT_SOURCE_DIR}/f3d_config.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/f3d_config.cxx"
  @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DConfigure.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkF3DConfigure.h"
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
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DConsoleOutputWindow.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DGenericImporter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DInteractorEventRecorder.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DInteractorStyle.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DLog.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DMetaReader.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DObjectFactory.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DOpenGLGridMapper.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DPolyDataMapper.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DPostProcessFilter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DRenderPass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DRenderer.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DRendererWithColoring.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DAnimationManager.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DOffscreenRender.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DOptions.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DStarter.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/f3d_config.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_interactor.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_loader.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_log.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_options.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_window.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_windowStandard.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_windowNoRender.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/readers/F3DReader.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/readers/F3DReaderFactory.cxx
  ${MODULES_SPECIFIC_SOURCE_FILES}
  ${LIB_PLATFORM_SPECIFIC_FILES}
  )

# List of headers that will be installed
set(F3D_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_interactor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_loader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_options.h
  )

add_library(libf3d
  ${F3D_SOURCE_FILES}
  )

target_include_directories(libf3d
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/readers>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/readers>
  $<BUILD_INTERFACE:${MODULES_SPECIFIC_INCLUDE_DIRS}>
  )

set_target_properties(libf3d PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  WINDOWS_EXPORT_ALL_SYMBOLS ON
  OUTPUT_NAME "f3d"
  )

target_compile_features(libf3d PRIVATE cxx_std_17)

if(F3D_INSTALL_SDK)
  target_include_directories(libf3d
    INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

  set_target_properties(libf3d PROPERTIES
    PUBLIC_HEADER "${F3D_PUBLIC_HEADERS}"
    )
endif()

vtk_module_autoinit(TARGETS libf3d MODULES ${F3D_VTK_MODULES})

target_link_libraries(libf3d PRIVATE ${F3D_VTK_MODULES} ${MODULES_SPECIFIC_LIBRARIES})

if(F3D_STRICT_BUILD)
  if(MSVC)
    # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
    target_compile_options(libf3d PRIVATE /W4 /WX /wd4275)

    # There are warnings in VTK related to deprecated features in C++17
    target_compile_definitions(libf3d PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
  else()
    target_compile_options(libf3d PRIVATE -Wall -Wextra -Wshadow -Werror)
  endif()
endif()

# coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
if(F3D_COVERAGE)
  target_compile_options(libf3d PUBLIC -g -O0 --coverage)
  target_link_options(libf3d PUBLIC --coverage)
endif()

# sanitizer
if(NOT F3D_SANITIZER)
  set(F3D_SANITIZER "none" CACHE STRING "Sanitizer type" FORCE)
  set_property(CACHE F3D_SANITIZER PROPERTY STRINGS "none" "address" "thread" "leak" "memory" "undefined")
endif()

if(NOT UNIX)
  set_property(CACHE F3D_SANITIZER PROPERTY TYPE INTERNAL)
endif()

if(NOT F3D_SANITIZER STREQUAL "none")
  target_compile_options(libf3d PUBLIC -fsanitize=${F3D_SANITIZER} -fno-optimize-sibling-calls -fno-omit-frame-pointer -g)
  if(${F3D_SANITIZER} STREQUAL "address")
    target_compile_options(libf3d PUBLIC -fsanitize-address-use-after-scope)
  endif()
  if(${F3D_SANITIZER} STREQUAL "memory")
    target_compile_options(libf3d PUBLIC -fsanitize-memory-track-origins)
  endif()
  target_link_options(libf3d PUBLIC -fsanitize=${F3D_SANITIZER})
endif()
