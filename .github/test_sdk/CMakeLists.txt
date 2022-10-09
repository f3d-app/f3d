project(test_sdk)

cmake_minimum_required(VERSION 3.2)

find_package(f3d REQUIRED)

# Check correctness of variables returned by f3dConfig.cmake
function(check_variable_defined f3d_var)
  if(NOT DEFINED ${f3d_var})
    message(FATAL_ERROR "${f3d_var} is not defined")
  endif()
  message(STATUS "${f3d_var} is set to ${${f3d_var}}")
endfunction()

check_variable_defined(f3d_MODULE_EXTERNAL_RENDERING)
check_variable_defined(f3d_MODULE_RAYTRACING)
check_variable_defined(f3d_MODULE_EXODUS)
check_variable_defined(f3d_MODULE_OCCT)
check_variable_defined(f3d_MODULE_ASSIMP)
check_variable_defined(f3d_MODULE_ALEMBIC)
check_variable_defined(f3d_BINDINGS_PYTHON)
check_variable_defined(f3d_BINDINGS_JAVA)

add_executable(test_sdk main.cxx)
target_link_libraries(test_sdk f3d::libf3d)

# make sure the libf3d API is compatible with C++11
set_target_properties(test_sdk PROPERTIES CXX_STANDARD 11)
