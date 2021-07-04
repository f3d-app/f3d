# Generate F3DIcon.h
add_executable(embed embed.cxx)
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

# f3d
cmake_dependent_option(F3D_MACOS_BUNDLE "Build a macOS bundle application" ON "APPLE" OFF)
cmake_dependent_option(F3D_WINDOWS_GUI "Build a non-console Win32 application" ON "WIN32" OFF)

set(PLATFORM_SPECIFIC_FILES "")
if(WIN32)
  set(PLATFORM_SPECIFIC_FILES "${CMAKE_SOURCE_DIR}/resources/f3d.rc")
  if(F3D_WINDOWS_GUI)
    set(PLATFORM_SPECIFIC_FILES ${PLATFORM_SPECIFIC_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DWin32OutputWindow.cxx")
  endif()
endif()
if(APPLE)
  set(PLATFORM_SPECIFIC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm")
endif()

set(MODULES_SPECIFIC_SOURCE_FILES "")
set(MODULES_SPECIFIC_INCLUDE_DIRS "")
set(MODULES_SPECIFIC_LIBRARIES "")
set(F3D_MODULE_OCCT_XCAF 0)
if(F3D_MODULE_OCCT)
  find_package(OpenCASCADE REQUIRED)
  if(OpenCASCADE_FOUND)
    if((TARGET "TKXDESTEP") AND (TARGET "TKXDEIGES"))
      set(F3D_MODULE_OCCT_XCAF 1)
    else()
      message(WARNING "OpenCASCADE: TKXDESTEP and TKXDEIGES are not found. Parts color and name are not supported.")
    endif()
    if((NOT TARGET "TKSTEP") AND (NOT TARGET "TKIGES") AND (NOT TARGET "TKMesh"))
      message(FATAL_ERROR "OpenCASCADE does not contain required modules")
    endif()
    set(MODULES_SPECIFIC_SOURCE_FILES ${MODULES_SPECIFIC_SOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DOCCTReader.cxx")
    set(MODULES_SPECIFIC_INCLUDE_DIRS ${MODULES_SPECIFIC_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR})
    # Note: OpenCASCADE does not declare the libs in the correct dependency order.
    # This could be an issue on Linux when linking with static libraries.
    set(MODULES_SPECIFIC_LIBRARIES ${MODULES_SPECIFIC_LIBRARIES} ${OpenCASCADE_LIBRARIES})
  endif()
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.h"
  @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.cxx"
  @ONLY)

# Readers declaration
macro(declare_f3d_reader reader)
  list(APPEND F3D_READER_FILES "readers/${reader}.h")
  set(F3D_READER_INCLUDES_CODE
    "${F3D_READER_INCLUDES_CODE}#include \"${reader}.h\"\n")
  set(F3D_READER_INSTANCES_CODE
    "${F3D_READER_INSTANCES_CODE}  ${reader} InstanceOf${reader};\n")
  set(F3D_READER_REGISTER_CODE
    "${F3D_READER_REGISTER_CODE}    F3DReaderFactory::Register(&InstanceOf${reader});\n")
  set(F3D_READER_UNREGISTER_CODE
    "${F3D_READER_UNREGISTER_CODE}    F3DReaderFactory::Unregister(&InstanceOf${reader});\n")
endmacro()

declare_f3d_reader(F3D3DSReader)
declare_f3d_reader(F3DCityGMLReader)
declare_f3d_reader(F3DDICOMReader)
declare_f3d_reader(F3DGLTFReader)
declare_f3d_reader(F3DMetaImageReader)
declare_f3d_reader(F3DNrrdReader)
declare_f3d_reader(F3DOBJReader)
declare_f3d_reader(F3DPLYReader)
declare_f3d_reader(F3DPTSReader)
declare_f3d_reader(F3DSTLReader)
declare_f3d_reader(F3DTIFFReader)
declare_f3d_reader(F3DVRMLReader)
declare_f3d_reader(F3DVTKLegacyReader)
declare_f3d_reader(F3DVTKXMLReader)

if(F3D_MODULE_OCCT)
  declare_f3d_reader(F3DOpenCascadeReader)
endif()

if(F3D_MODULE_EXODUS)
  declare_f3d_reader(F3DExodusIIReader)
endif()

# Generate the code of the F3DReaderInstantiator class
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/F3DReaderInstantiator.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/F3DReaderInstantiator.h"
  @ONLY)

list(LENGTH F3D_READER_FILES F3D_NB_READERS)
message(STATUS "${F3D_NB_READERS} readers found.")

# f3d executable

set(F3D_SOURCE_FILES
  main.cxx
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
  ${CMAKE_CURRENT_BINARY_DIR}/Config.cxx
  ${PLATFORM_SPECIFIC_FILES}
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
  F3DOffscreenRender.cxx
  F3DOptions.cxx
  readers/F3DReader.h
  readers/F3DReaderFactory.h
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.h
  ${CMAKE_CURRENT_BINARY_DIR}/F3DReaderInstantiator.h
  )

add_executable(f3d
  ${F3D_SOURCE_FILES}
  ${F3D_HEADER_FILES}
  ${F3D_READER_FILES}
  ${PLATFORM_SPECIFIC_FILES}
  ${MODULES_SPECIFIC_SOURCE_FILES}
  )

target_include_directories(f3d PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/readers
  )

set_target_properties(f3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warning and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)
if(F3D_STRICT_BUILD)
  if(MSVC)
    # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
    target_compile_options(f3d PRIVATE /W4 /WX /wd4275)
  else()
    target_compile_options(f3d PRIVATE -Wall -Wextra -Werror)
  endif()
endif()

# coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
if(F3D_COVERAGE)
  target_compile_options(f3d PRIVATE -g -O0 --coverage)
  target_link_options(f3d PRIVATE --coverage)
endif()

target_include_directories(f3d
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${MODULES_SPECIFIC_INCLUDE_DIRS})

target_compile_definitions(f3d
  PRIVATE VTK_VERSION_MAJOR=${VTK_VERSION_MAJOR}
  PRIVATE VTK_VERSION_MINOR=${VTK_VERSION_MINOR}
)

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

vtk_module_autoinit(TARGETS f3d MODULES ${F3D_VTK_MODULES})
target_link_libraries(f3d PUBLIC ${F3D_VTK_MODULES} ${MODULES_SPECIFIC_LIBRARIES})

install(TARGETS f3d
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ".")
