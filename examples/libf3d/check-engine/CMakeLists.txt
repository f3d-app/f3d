cmake_minimum_required(VERSION 3.5)

project(check-engine)

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
check_variable_defined(f3d_MODULE_EXR)
check_variable_defined(f3d_BINDINGS_PYTHON)
check_variable_defined(f3d_BINDINGS_JAVA)

add_executable(check-engine main.cxx)
target_link_libraries(check-engine f3d::libf3d)

# make sure the libf3d API is compatible with C++11
set_target_properties(check-engine PROPERTIES CXX_STANDARD 11)

if(BUILD_TESTING)
  enable_testing()
  add_test(NAME test_check-engine COMMAND "$<TARGET_FILE:check-engine>")
endif()
