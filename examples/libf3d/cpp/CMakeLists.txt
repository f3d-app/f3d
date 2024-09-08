find_package(f3d REQUIRED COMPONENTS library)

if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(check-engine)
add_subdirectory(multi-geom)
add_subdirectory(render-image)
add_subdirectory(render-interact)
add_subdirectory(use-options-string)
add_subdirectory(use-options-struct)
add_subdirectory(use-options-variant)
