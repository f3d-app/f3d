set(vtkextTests_list)

# Also needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
# Sanitizer exclusion because of https://github.com/f3d-app/f3d/issues/1323
if(NOT ANDROID AND NOT EMSCRIPTEN AND NOT F3D_SANITIZER STREQUAL "address")
  list(APPEND vtkextTests_list
       TestF3DBitonicSort.cxx)
endif()

vtk_add_test_cxx(vtkextTests tests
  NO_DATA NO_VALID NO_OUTPUT
  ${vtkextTests_list}
  ${F3D_SOURCE_DIR}/testing/ ${CMAKE_BINARY_DIR}/Testing/Temporary/)
vtk_test_cxx_executable(vtkextTests tests)

foreach(test ${vtkextTests_list})
  get_filename_component (TName ${test} NAME_WE)
  set_tests_properties(f3d::vtkextCxx-${TName} PROPERTIES
          LABELS "module"
  )
endforeach()

if(NOT ANDROID AND NOT EMSCRIPTEN AND NOT F3D_SANITIZER STREQUAL "address")
  set_target_properties(vtkextTests PROPERTIES CXX_STANDARD 20)
endif()
