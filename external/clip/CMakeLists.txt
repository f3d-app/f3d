# Clip Library
# Copyright (c) 2015-2025 David Capello

cmake_minimum_required(VERSION 3.15)

project(clip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  # Use libc++ explicitly so we can compile for
  # CMAKE_OSX_DEPLOYMENT_TARGET=10.7 or 10.8
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

option(CLIP_ENABLE_IMAGE "Compile with support to copy/paste images" on)
if(WIN32)
  option(CLIP_ENABLE_LIST_FORMATS "Compile with support to list clipboard formats" off)
endif()
option(CLIP_EXAMPLES "Compile clip examples" on)
option(CLIP_TESTS "Compile clip tests" on)
option(CLIP_INSTALL "Enable clip installation" on)
if(UNIX AND NOT APPLE)
  option(CLIP_X11_WITH_PNG "Compile with libpng to support copy/paste image in png format" on)
endif()

add_library(clip clip.cpp)

if(CLIP_ENABLE_IMAGE)
  target_sources(clip PRIVATE image.cpp)
  target_compile_definitions(clip PUBLIC -DCLIP_ENABLE_IMAGE=1)
endif()

if(WIN32 AND CLIP_ENABLE_LIST_FORMATS)
  target_compile_definitions(clip PUBLIC -DCLIP_ENABLE_LIST_FORMATS=1)
endif()

if(WIN32)
  option(CLIP_SUPPORT_WINXP "Enable Windows XP support" OFF)

  target_sources(clip PRIVATE clip_win.cpp)
  if(CLIP_ENABLE_IMAGE)
    target_sources(clip PRIVATE clip_win_bmp.cpp clip_win_wic.cpp)
    target_link_libraries(clip shlwapi)
  endif()

  if(MSVC)
    target_compile_definitions(clip PRIVATE -D_SCL_SECURE_NO_WARNINGS)
  endif()
  if (CLIP_SUPPORT_WINXP)
    target_compile_definitions(clip PRIVATE -DCLIP_SUPPORT_WINXP)
  endif()

  # MinGW requires the windowscodecs just because CLSIDs are defined
  # in the windowscodecs.a file instead of the wincodec.h file (?!)
  if(MINGW)
    find_library(CLIP_WINDOWSCODECS_LIBRARY windowscodecs)
    if(CLIP_WINDOWSCODECS_LIBRARY)
      target_link_libraries(clip ${CLIP_WINDOWSCODECS_LIBRARY})
    endif()
  endif()
elseif(APPLE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(clip PRIVATE -fobjc-arc)
  endif()

  find_library(COCOA_LIBRARY Cocoa REQUIRED)

  target_sources(clip PRIVATE clip_osx.mm)
  target_link_libraries(clip ${COCOA_LIBRARY})
elseif(UNIX AND NOT EMSCRIPTEN)
  include(CheckIncludeFiles)
  check_include_files(xcb/xcb.h HAVE_XCB_XLIB_H)

  if(NOT HAVE_XCB_XLIB_H)
    message(FATAL_ERROR "xcb/xcb.h not found, install libxcb-dev package")
  endif()

  target_compile_definitions(clip PRIVATE -DHAVE_XCB_XLIB_H)
  target_link_libraries(clip xcb pthread)

  if(CLIP_ENABLE_IMAGE AND CLIP_X11_WITH_PNG)
    check_include_files(png.h HAVE_PNG_H)
    if(CLIP_X11_PNG_LIBRARY)
      set(PNG_LIBRARY ${CLIP_X11_PNG_LIBRARY})
    else()
      find_library(PNG_LIBRARY png)
    endif()
    if(HAVE_PNG_H AND PNG_LIBRARY)
      target_compile_definitions(clip PRIVATE -DHAVE_PNG_H)
    endif()
    target_link_libraries(clip ${PNG_LIBRARY})
  endif()
  target_sources(clip PRIVATE clip_x11.cpp)
else()
  target_sources(clip PRIVATE clip_none.cpp)
endif()

if(CLIP_EXAMPLES)
  add_subdirectory(examples)
endif()

if(CLIP_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(CLIP_INSTALL)
  include(GNUInstallDirs)

  install(
    FILES clip.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(
    TARGETS clip
    EXPORT clip-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  install(
    EXPORT clip-targets
    NAMESPACE clip::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clip
  )

  include(CMakePackageConfigHelpers)

  configure_package_config_file(
    clip-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/clip-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clip
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/clip-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clip
  )
endif()
