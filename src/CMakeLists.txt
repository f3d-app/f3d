# Shared options between application and library
cmake_dependent_option(F3D_WINDOWS_GUI "Build a non-console Win32 application" ON "WIN32" OFF)
cmake_dependent_option(F3D_MACOS_BUNDLE "Build a macOS bundle application" ON "APPLE" OFF)

# Force static library when creating a macOS bundle
cmake_dependent_option(BUILD_SHARED_LIBS "Build f3d with shared libraries" ON "NOT F3D_MACOS_BUNDLE" OFF)

# F3D_STRICT_BUILD
set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warnings and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)
if(MSVC)
  # Warning C4275 is essentially noise, disabling it to silent an issue with jsoncpp library
  set(f3d_strict_build_compile_options /W4 /WX /wd4275)
else()
  set(f3d_strict_build_compile_options -Wall -Wextra -Wshadow -Woverloaded-virtual -Wno-deprecated -Wno-strict-overflow -Wno-array-bounds -Wunreachable-code -Wno-missing-field-initializers -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls -Wpointer-arith -Werror)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND f3d_strict_build_compile_options -Wsuggest-override)
  endif()
endif()

# Coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
if(F3D_COVERAGE)
  set(f3d_coverage_compile_options -g -O0 --coverage)
  set(f3d_coverage_link_options --coverage)
endif()

# Sanitizer
if(NOT F3D_SANITIZER)
  set(F3D_SANITIZER "none" CACHE STRING "Sanitizer type" FORCE)
  set_property(CACHE F3D_SANITIZER PROPERTY STRINGS "none" "address" "thread" "leak" "memory" "undefined")
endif()

if(NOT UNIX)
  set_property(CACHE F3D_SANITIZER PROPERTY TYPE INTERNAL)
endif()

if(NOT F3D_SANITIZER STREQUAL "none")
  set(sanitizer_compile_options -fsanitize=${F3D_SANITIZER} -fno-optimize-sibling-calls -fno-omit-frame-pointer -g)
  if(${F3D_SANITIZER} STREQUAL "address")
    list(APPEND sanitizer_compile_options -fsanitize-address-use-after-scope)
  endif()
  if(${F3D_SANITIZER} STREQUAL "memory")
    list(APPEND sanitizer_compile_options -fsanitize-memory-track-origins)
  endif()
  set(sanitizer_link_options -fsanitize=${F3D_SANITIZER})
endif()

# libf3d target
add_subdirectory(library)

# f3d target
add_subdirectory(application)

if(BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
  add_subdirectory(winshellext)
endif()

# python bindings
if(F3D_PYTHON_BINDINGS)
  add_subdirectory(python)
endif()
