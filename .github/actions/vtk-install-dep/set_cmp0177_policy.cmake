# CMake 3.31 trigger a warning without this policy set
if(POLICY CMP0174)
  cmake_policy(SET CMP0174 OLD)
endif()
