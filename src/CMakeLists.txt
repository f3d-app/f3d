# Generate F3DIcon.h
add_executable(embed embed.cxx)
target_link_libraries(embed PUBLIC VTK::IOImage)

if(WIN32)
  get_target_property(_confs VTK::IOImage IMPORTED_CONFIGURATIONS)
  foreach(_conf ${_confs})
    get_target_property(_runtime_path VTK::IOImage IMPORTED_LOCATION_${_conf})
    get_filename_component(_runtime_dir ${_runtime_path} DIRECTORY)
    list(APPEND CMAKE_MSVCIDE_RUN_PATH "${_runtime_dir}")
  endforeach()
endif()

add_custom_command(
  OUTPUT F3DIcon.h
  COMMAND $<TARGET_FILE:embed>
    "${CMAKE_CURRENT_SOURCE_DIR}/../resources/logo32.png"
    "${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.h"
  DEPENDS embed)

# f3d
if(WIN32)
  set(WINDOWS_RC_FILE ${CMAKE_SOURCE_DIR}/resources/f3d.rc)
endif()

if(APPLE)
  set(OSX_DELEGATE ${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm)
endif()

add_executable(f3d
  main.cxx
  vtkF3DGenericImporter.cxx
  vtkF3DInteractorStyle.cxx
  vtkF3DMetaReader.cxx
  vtkF3DObjectFactory.cxx
  vtkF3DOpenGLGridMapper.cxx
  vtkF3DPolyDataMapper.cxx
  vtkF3DPostProcessFilter.cxx
  vtkF3DRenderPass.cxx
  vtkF3DRenderer.cxx
  F3DAnimationManager.cxx
  F3DLoader.cxx
  F3DLog.cxx
  F3DOffscreenRender.cxx
  F3DOptions.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.h
  ${CMAKE_CURRENT_BINARY_DIR}/Config.cxx
  ${OSX_DELEGATE}
  ${WINDOWS_RC_FILE}
)

set_target_properties(f3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warning and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)
if (F3D_STRICT_BUILD)
  if(MSVC)
    target_compile_options(f3d PRIVATE /W4 /WX)
  else()
    target_compile_options(f3d PRIVATE -Wall -Wextra -Werror)
  endif()
endif()

target_include_directories(f3d
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(f3d
  PRIVATE VTK_VERSION_MAJOR=${VTK_VERSION_MAJOR}
  PRIVATE VTK_VERSION_MINOR=${VTK_VERSION_MINOR}
)

if(APPLE)
  # MacOS Bundle app
  option(MACOSX_BUILD_BUNDLE ON "Build a macOS bundle application")
  if(MACOSX_BUILD_BUNDLE)
    set_target_properties(f3d PROPERTIES MACOSX_BUNDLE TRUE)
    set(F3D_OSX_BUNDLE ON)

    # Add default configuration
    if(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
      set(f3d_CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json)
      set_source_files_properties(${f3d_CONFIG} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
      target_sources(f3d PRIVATE ${f3d_CONFIG})
    endif()
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

if(WIN32)
  # Windows executable without console
  option(WINDOWS_BUILD_WIN32 ON "Build a non-console Win32 application")
  if(WINDOWS_BUILD_WIN32)
    set_target_properties(f3d PROPERTIES WIN32_EXECUTABLE TRUE)
    set(F3D_WIN32_APP ON)
  endif()
endif()

set(F3D_MODULES
  VTK::FiltersGeneral
  VTK::FiltersGeometry
  VTK::ImagingCore
  VTK::ImagingHybrid
  VTK::InteractionStyle
  VTK::InteractionWidgets
  VTK::IOCityGML
  VTK::IOExodus
  VTK::IOGeometry
  VTK::IOImage
  VTK::IOImport
  VTK::IOParallel
  VTK::IOPLY
  VTK::IOXML
  VTK::RenderingAnnotation
  VTK::RenderingCore
  VTK::RenderingLabel
  VTK::RenderingOpenGL2
  VTK::RenderingVolumeOpenGL2
  VTK::jsoncpp
  VTK::opengl
)

if(TARGET VTK::RenderingRayTracing)
  message(STATUS "VTK RayTracing enabled")
  set(F3D_HAS_RAYTRACING ON CACHE INTERNAL BOOL)
  set(F3D_MODULES ${F3D_MODULES} VTK::RenderingRayTracing)
else()
  set(F3D_HAS_RAYTRACING OFF CACHE INTERNAL BOOL)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.h"
  @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.cxx"
  @ONLY)

vtk_module_autoinit(TARGETS f3d MODULES ${F3D_MODULES})
target_link_libraries(f3d PUBLIC ${F3D_MODULES})

install(TARGETS f3d
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ".")
