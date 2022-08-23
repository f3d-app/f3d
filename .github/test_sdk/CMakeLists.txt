project(test_sdk)

cmake_minimum_required(VERSION 3.2)

find_package(f3d REQUIRED)

add_executable(test_sdk main.cxx)
target_link_libraries(test_sdk f3d::libf3d)

# make sure the libf3d API is compatible with C++11
set_target_properties(test_sdk PROPERTIES CXX_STANDARD 11)
