---
description: 'C++ project configuration and package management'
applyTo: '**/*.cmake, **/CMakeLists.txt, **/*.cpp, **/*.h, **/*.hpp'
---

This project uses vcpkg in manifest mode. Please keep this in mind when giving vcpkg suggestions. Do not provide suggestions like vcpkg install library, as they will not work as expected.
Prefer setting cache variables and other types of things through CMakePresets.json if possible.
Give information about any CMake Policies that might affect CMake variables that are suggested or mentioned.
This project needs to be cross-platform and cross-compiler for MSVC, Clang (+AppleClang), and GCC.
