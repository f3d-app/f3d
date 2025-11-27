cmake_minimum_required(VERSION 3.10)

project(minimal_fltk)

if (POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

find_package(f3d REQUIRED COMPONENTS library)
find_package(FLTK REQUIRED CONFIG)

add_executable(minimal_fltk main.cxx)

target_link_libraries(minimal_fltk
  PRIVATE
  f3d::libf3d
  ${FLTK_LIBRARIES}
)

set_target_properties(minimal_fltk PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)
