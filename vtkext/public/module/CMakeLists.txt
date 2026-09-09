set(shader_files "")
set(sources "")
set(private_headers "")

if(NOT F3D_USE_GLES)
  set(shader_files ${shader_files}
    glsl/vtkF3DBitonicSortGlobalDisperseCS.glsl
    glsl/vtkF3DBitonicSortGlobalFlipCS.glsl
    glsl/vtkF3DBitonicSortLocalDisperseCS.glsl
    glsl/vtkF3DBitonicSortLocalSortCS.glsl
    glsl/vtkF3DBitonicSortFunctions.glsl)
endif()

foreach(file IN LISTS shader_files)
  vtk_encode_string(
    INPUT         "${file}"
    HEADER_OUTPUT header
    SOURCE_OUTPUT source)
  list(APPEND sources
    "${source}")
  list(APPEND private_headers
    "${header}")
endforeach()

set(classes
  F3DUtils
  vtkF3DArchiveReader
  vtkF3DCatmullClarkSubdivisionFilter
  vtkF3DFaceVaryingPointDispatcher
  vtkF3DGLTFImporter
  vtkF3DImporter
  )

if(NOT F3D_USE_EXTERNAL_MINIZ)
  list(APPEND sources "${F3D_SOURCE_DIR}/external/miniz/miniz.c")
endif()

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
if(NOT F3D_USE_GLES)
  set(classes ${classes} vtkF3DBitonicSort)
endif()

vtk_module_add_module(f3d::vtkext
  ${f3d_vtk_no_install}
  ${f3d_vtk_force_static}
  CLASSES ${classes}
  SOURCES ${sources}
  PRIVATE_HEADERS ${private_headers}
  )

if(F3D_USE_EXTERNAL_MINIZ)
  vtk_module_link(f3d::vtkext PRIVATE miniz::miniz)
else()
  vtk_module_include(f3d::vtkext
    PRIVATE
      $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/miniz>)
endif()
