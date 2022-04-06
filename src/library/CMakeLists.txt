# Generate F3DIcon buffer
vtk_encode_string(
  INPUT "${CMAKE_SOURCE_DIR}/resources/logo32.png"
  NAME F3DIcon
  BINARY)

# Define VTK modules to link libf3d with
# Note that readers/importers specific modules
# should be listed here as reader instanciation is header only
set(F3D_VTK_MODULES
  VTK::CommonSystem
  VTK::IOCityGML
  VTK::IOGeometry
  VTK::IOPLY
  VTK::IOXML
  VTK::IOParallel
  VTK::InteractionWidgets
  VTK::jsoncpp
  f3d::VTKExtensionsApplicative
  f3d::VTKExtensionsCore
  f3d::VTKExtensionsReaders
  f3d::VTKExtensionsRendering
)

# Check modules
if(F3D_MODULE_EXODUS)
  if(TARGET VTK::IOExodus)
    message(STATUS "Module: VTK ExodusII found")
    set(F3D_VTK_MODULES ${F3D_VTK_MODULES} VTK::IOExodus)
  else()
    message(FATAL_ERROR "F3D_MODULE_EXODUS option is enabled but VTK::IOExodus is not found")
  endif()
endif()

if(F3D_MODULE_RAYTRACING)
  if(TARGET VTK::RenderingRayTracing)
    message(STATUS "VTK RayTracing found")
  else()
    message(FATAL_ERROR "F3D_MODULE_RAYTRACING option is enabled but VTK::RenderingRayTracing is not found")
  endif()
endif()

if(F3D_MODULE_ASSIMP)
  find_package(assimp 5.0 REQUIRED)
  if(assimp_FOUND)
    set(F3D_ASSIMP_VERSION "${assimp_VERSION}")
    message(STATUS "Module: assimp ${F3D_ASSIMP_VERSION} found")
    if("${F3D_ASSIMP_VERSION}" VERSION_GREATER_EQUAL "5.1.0")
      message(WARNING "Module: assimp: Animations are not working with assimp 5.1.0 and newer, use assimp 5.0.X for animation support with assimp formats.")
    endif()
    if("${F3D_ASSIMP_VERSION}" VERSION_LESS "5.1.0")
      message(WARNING "Module: assimp: Embedded texture are only working with assimp 5.1.X and newer.")
    endif()
  endif()
endif()

set(F3D_MODULE_OCCT_XCAF 0)
if(F3D_MODULE_OCCT)
  find_package(OpenCASCADE REQUIRED)
  if(OpenCASCADE_FOUND)
    set(F3D_OCCT_VERSION "${OpenCASCADE_VERSION}")
    message(STATUS "Module: OpenCASCADE ${F3D_OCCT_VERSION} found")
    if((TARGET "TKXDESTEP") AND (TARGET "TKXDEIGES"))
      set(F3D_MODULE_OCCT_XCAF 1)
    else()
      message(WARNING "Module: OpenCASCADE: TKXDESTEP and TKXDEIGES are not found. Parts color and name are not supported.")
    endif()
    if((NOT TARGET "TKSTEP") AND (NOT TARGET "TKIGES") AND (NOT TARGET "TKMesh"))
      message(FATAL_ERROR "Module: OpenCASCADE does not contain required modules")
    endif()
  endif()
endif()

if(F3D_MODULE_ALEMBIC)
  find_package(Alembic 1.7 REQUIRED)
  if(Alembic_FOUND)
    set(F3D_ALEMBIC_VERSION "${Alembic_VERSION}")
    message(STATUS "Module: alembic ${Alembic_VERSION} found")
  endif()
endif()

set(libf3d_compile_options_public)
set(libf3d_compile_options_private)
set(libf3d_link_options_public)

if(F3D_STRICT_BUILD)
  if(MSVC)
    # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
    list(APPEND libf3d_compile_options_private /W4 /WX /wd4275)
  else()
    list(APPEND libf3d_compile_options_private -Wall -Wextra -Wshadow -Werror)
  endif()
endif()

# coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
if(F3D_COVERAGE)
  list(APPEND libf3d_compile_options_public -g -O0 --coverage)
  list(APPEND libf3d_link_options_public --coverage)
endif()

# sanitizer
if(NOT F3D_SANITIZER)
  set(F3D_SANITIZER "none" CACHE list "Sanitizer type" FORCE)
  set_property(CACHE F3D_SANITIZER PROPERTY STRINGS "none" "address" "thread" "leak" "memory" "undefined")
endif()

if(NOT UNIX)
  set_property(CACHE F3D_SANITIZER PROPERTY TYPE INTERNAL)
endif()

if(NOT F3D_SANITIZER STREQUAL "none")
  list(APPEND libf3d_compile_options_public -fsanitize=${F3D_SANITIZER} -fno-optimize-sibling-calls -fno-omit-frame-pointer -g)
  if(${F3D_SANITIZER} STREQUAL "address")
    list(APPEND libf3d_compile_options_public -fsanitize-address-use-after-scope)
  endif()
  if(${F3D_SANITIZER} STREQUAL "memory")
    list(APPEND libf3d_compile_options_public -fsanitize-memory-track-origins)
  endif()
  list(APPEND libf3d_link_options_public -fsanitize=${F3D_SANITIZER})
endif()

add_subdirectory(VTKExtensions)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/f3d_config.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/f3d_config.cxx"
  @ONLY)

# Define libf3d target
set(F3D_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DAnimationManager.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/F3DIcon.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DOptions.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/F3DStarter.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/f3d_config.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_init.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_engine.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_interactor_impl.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_loader_impl.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_log.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_options.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_window_impl.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_window_impl_standard.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_window_impl_noRender.cxx
  )

if(APPLE)
  list(APPEND F3D_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/F3DNSDelegate.mm")
endif()

# List of headers that will be installed
set(F3D_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_engine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_interactor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_loader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_log.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_options.h
  ${CMAKE_CURRENT_SOURCE_DIR}/f3d_window.h
  )

add_library(libf3d
  ${F3D_SOURCE_FILES}
  )

target_include_directories(libf3d
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
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

if(F3D_STRICT_BUILD)
  if(MSVC)
    # There are warnings in VTK related to deprecated features in C++17
    target_compile_definitions(libf3d PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
  endif()
endif()

target_link_libraries(libf3d PRIVATE ${F3D_VTK_MODULES})
target_compile_options(libf3d PUBLIC ${libf3d_compile_options_public} PRIVATE ${libf3d_compile_options_private})
target_link_options(libf3d PUBLIC ${libf3d_link_options_public})

# testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()
