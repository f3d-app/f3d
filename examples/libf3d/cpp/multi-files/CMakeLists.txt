cmake_minimum_required(VERSION 3.10)

project(multi-files)

find_package(f3d REQUIRED COMPONENTS library)

add_executable(multi-files main.cxx)
target_link_libraries(multi-files f3d::libf3d)
set_target_properties(multi-files PROPERTIES CXX_STANDARD 17)

if(UNIX AND NOT APPLE)
  target_link_libraries(multi-files stdc++fs)
endif()
