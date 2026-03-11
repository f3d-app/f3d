cmake_minimum_required(VERSION 3.10)

project(interactive-app-java)

find_package(Java REQUIRED)
include(UseJava)

find_package(f3d REQUIRED COMPONENTS library)

find_file(F3D_JAR
  NAMES f3d.jar
  PATHS
  ${f3d_DIR}/../../../share/java
  ${CMAKE_INSTALL_PREFIX}/share/java
  /usr/share/java
  /usr/local/share/java
  REQUIRED
)

message(STATUS "Found f3d.jar: ${F3D_JAR}")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF"
  "Manifest-Version: 1.0\n"
  "Main-Class: InteractiveApp\n"
  "Class-Path: ${F3D_JAR}\n"
)

set(CMAKE_JAVA_COMPILE_FLAGS -source 8 -target 8)
add_jar(interactive-app-java
  InteractiveApp.java
  INCLUDE_JARS ${F3D_JAR}
  OUTPUT_NAME interactive-app
  MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF"
)
