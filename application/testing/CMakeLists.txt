# F3D Testing
function(f3d_test)

  cmake_parse_arguments(F3D_TEST "TONE_MAPPING;LONG_TIMEOUT;INTERACTION;INTERACTION_CONFIGURE;NO_BASELINE;NO_RENDER;NO_OUTPUT;WILL_FAIL;NO_DATA_FORCE_RENDER;UI;PIPED" "NAME;CONFIG;RESOLUTION;THRESHOLD;REGEXP;REGEXP_FAIL;HDRI;SCRIPT;RENDERING_BACKEND;WORKING_DIR" "DATA;DEPENDS;ENV;ARGS" ${ARGN})

  if(F3D_TEST_CONFIG)
    list(APPEND F3D_TEST_ARGS "--config=${F3D_TEST_CONFIG}")
  else()
    list(APPEND F3D_TEST_ARGS "--no-config")
  endif()

  set(_f3d_test_data)
  if (F3D_TEST_DATA)
    foreach(_single_data ${F3D_TEST_DATA})
      list(APPEND _f3d_test_data "${F3D_SOURCE_DIR}/testing/data/${_single_data}")
    endforeach()
  endif()

  if(F3D_TEST_INTERACTION)
    list(APPEND F3D_TEST_ARGS "--interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
  else ()
    if(F3D_TEST_INTERACTION_CONFIGURE)
      configure_file("${F3D_SOURCE_DIR}/testing/recordings/${F3D_TEST_NAME}.log.in" "${CMAKE_BINARY_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
      list(APPEND F3D_TEST_ARGS "--interaction-test-play=${CMAKE_BINARY_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
    endif()
  endif()

  if (F3D_TEST_HDRI)
    list(APPEND F3D_TEST_ARGS "--hdri-file=${F3D_SOURCE_DIR}/testing/data/${F3D_TEST_HDRI}" "--hdri-ambient" "--hdri-skybox")
    set(F3D_TEST_LONG_TIMEOUT ON)
  endif()

  if (F3D_TEST_SCRIPT)
    list(APPEND F3D_TEST_ARGS "--command-script=${F3D_SOURCE_DIR}/testing/scripts/${F3D_TEST_SCRIPT}")
  endif()

  if(F3D_TEST_NO_RENDER)
    list(APPEND F3D_TEST_ARGS "--no-render" "--verbose=debug")
  else()
    if(F3D_TEST_RESOLUTION)
      list(APPEND F3D_TEST_ARGS "--resolution=${F3D_TEST_RESOLUTION}")
    else()
      list(APPEND F3D_TEST_ARGS "--resolution=300,300")
    endif()

    if(NOT F3D_TEST_NO_OUTPUT)
      list(APPEND F3D_TEST_ARGS "--output=${CMAKE_BINARY_DIR}/Testing/Temporary/${F3D_TEST_NAME}.png")
    endif()
  endif()

  if(NOT F3D_TEST_NO_BASELINE)
    list(APPEND F3D_TEST_ARGS "--reference=${F3D_SOURCE_DIR}/testing/baselines/${F3D_TEST_NAME}.png")

    if(F3D_TEST_THRESHOLD)
      list(APPEND F3D_TEST_ARGS "--reference-threshold=${F3D_TEST_THRESHOLD}")
    endif()
  endif()

  if(F3D_TEST_RENDERING_BACKEND)
    list(APPEND F3D_TEST_ARGS "--rendering-backend=${F3D_TEST_RENDERING_BACKEND}")
  else()
    # If no rendering backend is specified by the test, "auto" is used.
    # However, F3D_TESTING_FORCE_RENDERING_BACKEND can be used to force the rendering backend for these tests
    if(F3D_TESTING_FORCE_RENDERING_BACKEND)
      list(APPEND F3D_TEST_ARGS "--rendering-backend=${F3D_TESTING_FORCE_RENDERING_BACKEND}")
    endif()
  endif()

  if (F3D_TEST_PIPED)
    list(JOIN F3D_TEST_ARGS " " F3D_TEST_ARGS_JOINED)
    add_test(
      NAME "f3d::${F3D_TEST_NAME}"
      COMMAND ${CMAKE_COMMAND} 
        -DF3D_EXE:FILEPATH=$<TARGET_FILE:f3d> 
        -DF3D_PIPED_DATA=${_f3d_test_data}
        -DF3D_ARGS=${F3D_TEST_ARGS_JOINED} 
        -P ${CMAKE_CURRENT_SOURCE_DIR}/f3d_piped.cmake
        COMMAND_EXPAND_LISTS)
  else()
    add_test(NAME "f3d::${F3D_TEST_NAME}" COMMAND $<TARGET_FILE:f3d> ${_f3d_test_data} ${F3D_TEST_ARGS} COMMAND_EXPAND_LISTS)
  endif()

  set(_timeout "30")
  if(F3D_TEST_LONG_TIMEOUT)
    set(_timeout "120")
  endif()

  # sanitizer multipliers (multipliers are coming from ASan documentation)
  # "undefined" and "leak" have no overhead
  if(F3D_SANITIZER STREQUAL "address")
    math(EXPR _timeout "2 * ${_timeout}")
  endif()
  if(F3D_SANITIZER STREQUAL "thread")
    math(EXPR _timeout "15 * ${_timeout}")
  endif()
  if(F3D_SANITIZER STREQUAL "memory")
    math(EXPR _timeout "3 * ${_timeout}")
  endif()

  if(F3D_TEST_UI AND NOT F3D_MODULE_UI)
    # UI tests require ImGui
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
  endif()

  if(F3D_TEST_TONE_MAPPING AND VTK_VERSION VERSION_LESS 9.3.20240609)
    # After VTK 9.3.20240609, the tone mapping used in F3D is PBR Neutral
    # Testing tone mapping is now disabled because the reference image is different
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
  endif()

  if(NOT F3D_TESTING_ENABLE_RENDERING_TESTS)
    if(NOT F3D_TEST_NO_RENDER)
      set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
    endif()
  endif()
  if(NOT F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS)
    if(F3D_TEST_LONG_TIMEOUT)
      set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
    endif()
  endif()
  set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES TIMEOUT ${_timeout})

  if(F3D_TEST_WILL_FAIL)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES WILL_FAIL TRUE)
  endif()

  if(F3D_TEST_REGEXP)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "${F3D_TEST_REGEXP}")
  endif()

  if(F3D_TEST_REGEXP_FAIL)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES FAIL_REGULAR_EXPRESSION "${F3D_TEST_REGEXP_FAIL}")
  endif()

  if(F3D_TEST_DEPENDS)
    foreach(_single_depends ${F3D_TEST_DEPENDS})
      set_tests_properties(f3d::${_single_depends} PROPERTIES FIXTURES_SETUP f3d::${_single_depends}_FIXTURE)
      list(APPEND _depends_fixtures f3d::${_single_depends}_FIXTURE)
    endforeach()
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES FIXTURES_REQUIRED "${_depends_fixtures}")
  endif()

  if(F3D_TEST_WORKING_DIR)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES WORKING_DIRECTORY "${F3D_TEST_WORKING_DIR}")
  endif()

  set(f3d_test_env_vars ${F3D_TEST_ENV})
  list(APPEND f3d_test_env_vars "CTEST_F3D_PROGRESS_BAR=1")
  if (F3D_TEST_UI)
    list(APPEND f3d_test_env_vars "CTEST_F3D_CONSOLE_BADGE=1")
  endif ()
  if (F3D_TEST_NO_DATA_FORCE_RENDER)
    list(APPEND f3d_test_env_vars "CTEST_F3D_NO_DATA_FORCE_RENDER=1")
  endif ()

  set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES ENVIRONMENT
    "F3D_PLUGINS_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${f3d_test_env_vars}")

endfunction()

set (_outOfRangeDoubleStr "\
12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012\
34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234\
56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456\
78901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678\
90123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\
1234567890123456789012345678901234567890\
")

f3d_test(NAME TestPLY DATA suzanne.ply)
f3d_test(NAME TestOBJ DATA world.obj)
f3d_test(NAME TestSTL DATA suzanne.stl)
f3d_test(NAME TestVTU DATA dragon.vtu)
f3d_test(NAME TestVTP DATA cow.vtp)
f3d_test(NAME TestVTR DATA RectGrid2.vtr ARGS --scalar-coloring --roughness=1)
f3d_test(NAME TestVTS DATA bluntfin.vts)
f3d_test(NAME TestVTM DATA mb.vtm)
f3d_test(NAME TestVTK DATA cow.vtk)
f3d_test(NAME TestNRRD DATA beach.nrrd ARGS -s)
f3d_test(NAME TestVRMLImporter DATA bot2.wrl)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260103)
  f3d_test(NAME TestPipedOBJ DATA world.obj ARGS --force-reader=OBJ PIPED) # MTL and Textures not loaded
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260106)
  f3d_test(NAME TestPipedVTR DATA RectGrid2.vtr ARGS --scalar-coloring --roughness=1 --force-reader=VTKXMLVTR PIPED)
  f3d_test(NAME TestPipedVTU DATA dragon.vtu ARGS --force-reader=VTKXMLVTU PIPED)
  f3d_test(NAME TestPipedVTP DATA cow.vtp ARGS --force-reader=VTKXMLVTP PIPED)
  f3d_test(NAME TestPipedVTS DATA bluntfin.vts ARGS --force-reader=VTKXMLVTS PIPED)
  f3d_test(NAME TestPipedVTICell DATA waveletMaterial.vti ARGS -s --coloring-array=Material -c --roughness=1 --force-reader=VTKXMLVTI PIPED)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260103)
  f3d_test(NAME TestPipedVTK DATA cow.vtk ARGS --force-reader=VTKLegacy PIPED)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251104)
  f3d_test(NAME TestPipedSTL DATA suzanne.stl ARGS --force-reader=STL PIPED)
  if(NOT F3D_MACOS_BUNDLE)
    f3d_test(NAME TestDefaultConfigFilePipedSTL DATA suzanne.stl ARGS --force-reader=STL CONFIG config_build LONG_TIMEOUT UI PIPED)
  endif()
endif()


if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231102)
  f3d_test(NAME TestSPLAT DATA small.splat ARGS -osy --up=-Y --point-sprites-absolute-size --point-sprites-size=1)
  f3d_test(NAME TestSPZ DATA hornedlizard_small_d0.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1)
endif()

f3d_test(NAME TestQuakeMDL DATA zombie.mdl)
f3d_test(NAME TestQuakeMDLAnimationSimpleFrame DATA zombie.mdl ARGS --animation-time=0.5)
f3d_test(NAME TestQuakeMDLAnimationBetween DATA zombie.mdl ARGS --animation-time=1.42)
f3d_test(NAME TestQuakeMDLAnimationMulti DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5)
f3d_test(NAME TestQuakeMDLAnimationGroupFrame DATA flame_mixed.mdl ARGS --animation-indices=1 --animation-time=0.5)
f3d_test(NAME TestQuakeMDLAnimationLastFrame DATA flame_mixed.mdl ARGS --camera-position=90,0,0 --animation-time=0.65)
f3d_test(NAME TestQuakeMDLDisableAnimation DATA zombie.mdl soldier_animations.mdl ARGS --multi-file-mode=all --animation-time=0.5)
f3d_test(NAME TestQuakeMDLSkinIndex DATA armor.mdl ARGS -DQuakeMDL.skin_index=2)
f3d_test(NAME TestQuakeMDLSkinIndexNegative DATA armor.mdl ARGS -DQuakeMDL.skin_index=-1)
f3d_test(NAME TestQuakeMDLSkinIndexNonInteger DATA armor.mdl ARGS -DQuakeMDL.skin_index=1.5)
f3d_test(NAME TestQuakeMDLSkinIndexOutOfBounds DATA armor.mdl ARGS -DQuakeMDL.skin_index=4)
f3d_test(NAME TestQuakeMDLSkinIndexOverflow DATA armor.mdl ARGS -DQuakeMDL.skin_index=9223372036854775808)
f3d_test(NAME TestQuakeMDLGroupSkin DATA groupskin.mdl ARGS --animation-indices=1 --animation-time=0.3)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_test(NAME TestQuakeMDLActorCollection DATA zombie.mdl ARGS --scalar-coloring)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  f3d_test(NAME TestPipedPLY DATA suzanne.ply ARGS --force-reader=PLYReader PIPED)
  f3d_test(NAME TestPipedQuakeMDL DATA zombie.mdl ARGS --force-reader=QuakeMDL PIPED)
  f3d_test(NAME TestPipedSPLAT DATA small.splat ARGS -osy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --force-reader=Splat PIPED)
  f3d_test(NAME TestPipedSPZ DATA hornedlizard_small_d0.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --force-reader=SPZ PIPED)
  f3d_test(NAME TestPipedInvalid DATA suzanne.ply ARGS --force-reader=QuakeMDL PIPED REGEXP "Input stream could not be loaded" NO_BASELINE)
endif()

f3d_test(NAME TestNormalGlyphsEnable DATA suzanne.obj ARGS --normal-glyphs)

f3d_test(NAME TestGridX DATA suzanne.ply ARGS -g --up=+X)
f3d_test(NAME TestGridY DATA suzanne.ply ARGS -g --up=+Y)
f3d_test(NAME TestGridZ DATA suzanne.ply ARGS -g --up=+Z)
f3d_test(NAME TestGridUp123 DATA suzanne.ply ARGS -g --up=1,2,3)
f3d_test(NAME TestGridUp100 DATA suzanne.ply ARGS -g --up=1,0,0)
f3d_test(NAME TestGridUp000 DATA suzanne.ply ARGS -g --up=0,0,0)
f3d_test(NAME TestGridOptions DATA suzanne.ply ARGS -g --camera-elevation-angle=45 --grid-unit=2 --grid-subdivisions=3)
f3d_test(NAME TestGridAbsolute DATA f3d.vtp ARGS -g --up=-Y --camera-direction=-.5,+1,+1 --grid-absolute)
f3d_test(NAME TestGridClipping DATA offset-flat-box.glb ARGS -g --grid-absolute --camera-position=70,120,350)
f3d_test(NAME TestGridColor DATA suzanne.ply ARGS -g --grid-color=1,1,1)
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestAxis DATA suzanne.ply ARGS -x UI)
  f3d_test(NAME TestAxesCustomColor DATA suzanne.ply ARGS -x -g --x-color=1,0,0.643 --y-color=0,1,0.894 --z-color=0.89,1,0 UI)
endif()
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11209
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250513)
  f3d_test(NAME TestAxesGridEnable DATA suzanne.ply ARGS --axes-grid THRESHOLD 0.08) # Threshold required for MacOS due to line rendering differences
endif()
f3d_test(NAME TestBackfaceVisible DATA backface.vtp ARGS --backface-type=visible)
f3d_test(NAME TestBackfaceHidden DATA backface.vtp ARGS --backface-type=hidden)
f3d_test(NAME TestPointCloud DATA pointsCloud.vtp ARGS -o --point-sprites-size=20)
f3d_test(NAME TestPointCloudBar DATA pointsCloud.vtp ARGS -sob --point-sprites-size=20)
f3d_test(NAME TestPointCloudUG DATA pointsCloud.vtu ARGS -o --point-sprites-size=20)
f3d_test(NAME TestPointCloudVolume DATA bluntfin.vts ARGS -sob)
f3d_test(NAME TestPointCloudDefaultScene DATA pointsCloud.vtp ARGS --point-size=20)
f3d_test(NAME Test3DSImporter DATA iflamigm.3ds ARGS --up=+Z)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250925)
  f3d_test(NAME TestPiped3DSImporter DATA iflamigm.3ds ARGS --up=+Z --force-reader=3DS PIPED)
endif()
f3d_test(NAME TestScalars DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1)
f3d_test(NAME TestScalarsCell DATA f3d.vtp ARGS --scalar-coloring --coloring-by-cells --coloring-component=-2 --up=+Z)
f3d_test(NAME TestScalarsRange DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1 --coloring-range=0,1)
f3d_test(NAME TestScalarsWithBar DATA suzanne.ply ARGS -b -s --coloring-array=Normals --coloring-component=0)
f3d_test(NAME TestScalarsWithBarContinuity DATA suzanne.ply ARGS -b -s --coloring-array=Normals --coloring-component=0)
f3d_test(NAME TestGLTFImporter DATA f3d.glb)
f3d_test(NAME TestGLTFImporterWithAnimation DATA BoxAnimated.gltf ARGS --animation-time=2 --animation-progress)
f3d_test(NAME TestGLTFSkin DATA SimpleSkin.gltf)
f3d_test(NAME TestDicom DATA IM-0001-1983.dcm ARGS --scalar-coloring --roughness=1)
f3d_test(NAME TestMHD DATA HeadMRVolume.mhd ARGS --scalar-coloring --roughness=1)
f3d_test(NAME TestVTICell DATA waveletMaterial.vti ARGS -s --coloring-array=Material -c --roughness=1)
f3d_test(NAME TestSSAO DATA suzanne.ply ARGS -q)
f3d_test(NAME TestDepthPeeling DATA suzanne.ply ARGS -sp --opacity=0.9)
f3d_test(NAME TestBackground DATA suzanne.ply ARGS --background-color=0.8,0.2,0.9 THRESHOLD 0.1) # Threshold is needed for legacy image comparison for VTK 9.3
f3d_test(NAME TestGridWithDepthPeeling DATA suzanne.ply ARGS -gp --opacity=0.2)
f3d_test(NAME TestFilename DATA suzanne.ply ARGS -n UI)
f3d_test(NAME TestHDRIFilename DATA dragon.vtu ARGS --hdri-filename -f -j --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr UI LONG_TIMEOUT)
f3d_test(NAME TestFilenameHDRIFilename DATA dragon.vtu ARGS --hdri-filename -n -f -j --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr RESOLUTION 400,400 UI LONG_TIMEOUT)
f3d_test(NAME TestHDRIFilenameDefault DATA dragon.vtu ARGS --hdri-filename -f -j UI LONG_TIMEOUT)
f3d_test(NAME TestFilenameWhiteBg DATA suzanne.ply ARGS -n --background-color=1,1,1 UI)
f3d_test(NAME TestConsoleBadgeWarning DATA suzanne.ply ARGS --position=0 UI)
f3d_test(NAME TestConsoleBadgeQuiet DATA suzanne.ply ARGS --position=0 --verbose=quiet UI)
f3d_test(NAME TestCityGML DATA Part-4-Buildings-V4-one.gml)
f3d_test(NAME TestPTS DATA samplePTS.pts)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251208)
  f3d_test(NAME TestPipedCityGML DATA Part-4-Buildings-V4-one.gml ARGS --force-reader=CityGML PIPED)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  f3d_test(NAME TestPipedDicom DATA IM-0001-1983.dcm ARGS --scalar-coloring --roughness=1 --force-reader=DICOM PIPED)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250923)
  f3d_test(NAME TestPipedGLTFImporter DATA f3d.glb ARGS --force-reader=GLB PIPED)
  f3d_test(NAME TestPipedGLTFSkin DATA SimpleSkin.gltf ARGS --force-reader=GLTF PIPED)
endif()  

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251109)
  f3d_test(NAME TestPipedPTS DATA samplePTS.pts ARGS --force-reader=PTS PIPED)
endif()  

f3d_test(NAME TestColormap DATA IM-0001-1983.dcm ARGS --scalar-coloring --roughness=1 --colormap=0,1,0,0,1,0,1,0)
f3d_test(NAME TestOpacityMap DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-map=0.0,0.0,0.5,1.0,1.0,0.0 THRESHOLD 0.05) # Small rendering differences when using VTK 9.3.0
f3d_test(NAME TestOpacityMapFile DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/gaussian_opacity_map.png)
f3d_test(NAME TestOpacityMapFileNonExistent DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/nonexistent_opacity_map.png)
f3d_test(NAME TestOpacityMapFileInvalid DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/invalid.png)
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestCameraConfiguration DATA suzanne.obj ARGS --camera-position=0,0,-10 -x --camera-view-up=+X --camera-focal-point=1,0,0 --camera-view-angle=20 --camera-azimuth-angle=40 --camera-elevation-angle=-80 --camera-direction=12,34,56 --camera-zoom-factor=78 UI)
endif()
f3d_test(NAME TestCameraDirection DATA suzanne.obj ARGS --camera-direction=-xy+z)
f3d_test(NAME TestCameraClipping DATA checkerboard_colorful.obj CONFIG ${F3D_SOURCE_DIR}/testing/configs/checkerboard_colorful.json RESOLUTION 800,600)
f3d_test(NAME TestCameraOrthographic ARGS --camera-orthographic DATA cow.vtp)
f3d_test(NAME TestCameraZoomFactor DATA suzanne.obj ARGS --camera-direction=-1,-1,1 --camera-zoom-factor=1.5)
f3d_test(NAME TestToneMapping DATA suzanne.ply ARGS -t TONE_MAPPING)
f3d_test(NAME TestDepthPeelingToneMapping DATA suzanne.ply ARGS --opacity=0.9 -pt TONE_MAPPING)
f3d_test(NAME TestVolume DATA HeadMRVolume.mhd ARGS -v --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1)
f3d_test(NAME TestVolumeInverse DATA HeadMRVolume.mhd ARGS -vi --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 THRESHOLD 0.05) # Small rendering differences due to volume rendering
f3d_test(NAME TestVolumeMag DATA vase_4comp.vti ARGS -vb)
f3d_test(NAME TestVolumeComp DATA vase_4comp.vti ARGS -vb --coloring-component=3 LONG_TIMEOUT)
f3d_test(NAME TestVolumeDirect DATA vase_4comp.vti ARGS -vb --coloring-component=-2)
f3d_test(NAME TestVolumeCells DATA waveletArrays.vti ARGS -vb --coloring-by-cells)
f3d_test(NAME TestVolumeColoringArray DATA waveletArrays.vti ARGS -vb --coloring-array=Result LONG_TIMEOUT)
f3d_test(NAME TestTextureNormal DATA WaterBottle.glb ARGS --texture-normal=${F3D_SOURCE_DIR}/testing/data/normal.png --normal-scale=0.1)
f3d_test(NAME TestTextureMaterial DATA WaterBottle.glb ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/red_mod.jpg --roughness=1 --metallic=1)
f3d_test(NAME TestTextureMaterialWithOptions DATA WaterBottle.glb ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/red_mod.jpg --roughness=0.5 --metallic=0.5)
f3d_test(NAME TestTextureEmissive DATA WaterBottle.glb ARGS --texture-emissive=${F3D_SOURCE_DIR}/testing/data/red.jpg --emissive-factor=0.1,0.1,0.1)
f3d_test(NAME TestTextures DATA WaterBottle.glb ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/red.jpg --roughness=1 --metallic=1 --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo.png --texture-normal=${F3D_SOURCE_DIR}/testing/data/normal.png --texture-emissive=${F3D_SOURCE_DIR}/testing/data/red.jpg --emissive-factor=0.1,0.1,0.1)
f3d_test(NAME TestMetaDataImporter DATA BoxAnimated.gltf ARGS -m UI)
f3d_test(NAME TestMultiblockMetaData DATA mb.vtm ARGS -m UI)
f3d_test(NAME TestRemoveEmptyFileGroups DATA mb/mb_3_0.vtt mb/mb_0_0.vtu ARGS -n --remove-empty-file-groups UI)
f3d_test(NAME TestMultiFileMetaData DATA mb/recursive ARGS -m --multi-file-mode=all UI)
f3d_test(NAME TestMultiFileModeDir DATA mb ARGS -n --multi-file-mode=dir --recursive-dir-add UI)
set(MULTIFILES
  multi-file/cube-part1of2.glb multi-file/cube-part2of2.glb
  multi-file/cube-part1of3.glb multi-file/cube-part2of3.glb multi-file/cube-part3of3.glb
  multi-file/tet-part1of2.glb multi-file/tet-part2of2.glb
  multi-file/tet-part1of3.glb multi-file/tet-part2of3.glb multi-file/tet-part3of3.glb)
f3d_test(NAME TestMultiFileRegexA DATA ${MULTIFILES} ARGS -n "--multi-file-regex=(\\d+)of\\d" --camera-direction=-xyz UI)
f3d_test(NAME TestMultiFileRegexB DATA ${MULTIFILES} ARGS -n "--multi-file-regex=part(.+)" --camera-direction=-xyz UI)
f3d_test(NAME TestMultiFileRegexInvalid ARGS "--multi-file-regex=(.+" REGEXP "invalid regular expression: \"\\(.+")
f3d_test(NAME TestTIFF DATA f3d.tif ARGS -sy --up=-Y)
f3d_test(NAME TestLightIntensityBrighter DATA cow.vtp ARGS --light-intensity=5.0)
f3d_test(NAME TestLightIntensityDarker DATA cow.vtp ARGS --light-intensity=0.2)
f3d_test(NAME TestLightIntensityBrighterFullScene DATA WaterBottle.glb ARGS --light-intensity=5.0)
f3d_test(NAME TestLightIntensityDarkerFullScene DATA WaterBottle.glb ARGS --light-intensity=0.2)
f3d_test(NAME TestUTF8 DATA "(ノಠ益ಠ )ノ.vtp")
f3d_test(NAME TestFilenameCommasSpaces DATA "tetrahedron, with commas & spaces.stl")
f3d_test(NAME TestFont DATA suzanne.ply ARGS -n --font-file=${F3D_SOURCE_DIR}/testing/data/Crosterian.ttf UI)
f3d_test(NAME TestFontScale2 DATA suzanne.ply ARGS -n --font-scale=2 UI)
f3d_test(NAME TestFontScale3 DATA suzanne.ply ARGS -n --font-scale=3 UI)
f3d_test(NAME TestDefines DATA dragon.vtu ARGS -Dscene.up_direction=+Z --define=model.point_sprites.enable=on)
f3d_test(NAME TestDefinesInvalid DATA dragon.vtu ARGS -Dscene.up_direction+Z REGEXP "Could not parse a define" NO_BASELINE)
f3d_test(NAME TestDefinesInexistent DATA dragon.vtu ARGS -Dscene.up_director=+Z REGEXP "option from CLI options does not exists" NO_BASELINE)
f3d_test(NAME TestConfigReset DATA suzanne.stl ARGS -Rrender.grid.enable --reset=ui.axis CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json)
f3d_test(NAME TestConfigResetInexistent DATA suzanne.stl ARGS -Rrender.glid.enable REGEXP "option from CLI options does not exists" NO_BASELINE)
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestConfigImperative DATA dragon.vtu ARGS --axis=false CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json UI)
endif()
f3d_test(NAME TestAnimationIndicesSingle DATA soldier_animations.mdl ARGS --animation-indices=7 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestAnimationIndicesMulti DATA InterpolationTest.glb ARGS --animation-indices=7,6 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestAnimationIndexDeprecated DATA InterpolationTest.glb ARGS --animation-index=7 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestMultiFileAnimationIndices DATA InterpolationTest.glb BoxAnimated.gltf ARGS --animation-indices=9 --animation-time=0.85 --animation-progress --multi-file-mode=all)
f3d_test(NAME TestMultiFileAnimationNoneMulti DATA bot2.wrl InterpolationTest.glb ARGS --animation-indices=6 --animation-time=0.85 --multi-file-mode=all)
f3d_test(NAME TestMultiFileAnimationAnySingle DATA soldier_animations.mdl InterpolationTest.glb ARGS --animation-indices=13 --animation-time=0.85 --multi-file-mode=all --opacity=0.5)
f3d_test(NAME TestMultiFileAnimationNoAnimationSupport DATA f3d.glb world.obj ARGS --multi-file-mode=all --animation-time=2 --animation-progress)
f3d_test(NAME TestAnimationAutoplay DATA InterpolationTest.glb ARGS --animation-autoplay)
f3d_test(NAME TestMaxSizeBelow DATA suzanne.stl ARGS --max-size=1)
f3d_test(NAME TestMaxSizeAbove DATA WaterBottle.glb ARGS --max-size=0.2 REGEXP "file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestMaxSizeAboveMultiFile DATA suzanne.obj WaterBottle.glb ARGS --multi-file-mode=all --max-size=0.6 --blending --opacity=0.5)
f3d_test(NAME TestAlternativeOptionSyntax DATA WaterBottle.glb ARGS --max-size 0.2 REGEXP "file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestNonExistentFile DATA nonExistentFile.vtp ARGS --filename WILL_FAIL)
f3d_test(NAME TestUnsupportedFile DATA unsupportedFile.dummy ARGS --filename WILL_FAIL)
f3d_test(NAME TestComponentName DATA from_abq.vtu ARGS --scalar-coloring --coloring-scalar-bar --coloring-component=2)
f3d_test(NAME TestNoRender DATA dragon.vtu NO_RENDER)
f3d_test(NAME TestNoRenderWithOptions DATA dragon.vtu ARGS --hdri-ambient --axis NO_RENDER) # These options causes issues if not handled correctly
f3d_test(NAME TestNoFile NO_DATA_FORCE_RENDER UI)
f3d_test(NAME TestNoFileEmptyFileName ARGS --filename NO_DATA_FORCE_RENDER UI)
f3d_test(NAME TestMultiFile DATA mb/recursive ARGS --multi-file-mode=all)
f3d_test(NAME TestMultiFileRecursive DATA mb ARGS --multi-file-mode=all --recursive-dir-add)
f3d_test(NAME TestMultiFileColoring DATA mb/recursive ARGS --multi-file-mode=all -s --coloring-array=Polynomial -b)
f3d_test(NAME TestMultiFileVolume DATA multi ARGS --multi-file-mode=all -vsb --coloring-array=Scalars_)
f3d_test(NAME TestMultiFileColoringTexture DATA mb/recursive/mb_1_0.vtp mb/recursive/mb_2_0.vtp world.obj ARGS --multi-file-mode=all -sb --coloring-array=Normals --coloring-component=1)
f3d_test(NAME TestMultiFilePositionals DATA mb/recursive/mb_0_0.vtu mb/recursive/mb_1_0.vtp ARGS --multi-file-mode=all -s --coloring-array=Polynomial -b)
f3d_test(NAME TestMultiFileNonCoherentComponentNames DATA bluntfin.vts bluntfin_t.vtu ARGS --multi-file-mode=all --scalar-coloring --coloring-array=Momentum --coloring-component=2 --coloring-scalar-bar)
f3d_test(NAME TestMultiInputArg ARGS --input ${F3D_SOURCE_DIR}/testing/data/mb/recursive/mb_0_0.vtu ${F3D_SOURCE_DIR}/testing/data/mb/recursive/mb_1_0.vtp --multi-file-mode=all -s --coloring-array=Polynomial -b)
f3d_test(NAME TestMultiInputMultiArgs ARGS --input ${F3D_SOURCE_DIR}/testing/data/mb/recursive/mb_0_0.vtu --input ${F3D_SOURCE_DIR}/testing/data/mb/recursive/mb_1_0.vtp --multi-file-mode=all -s --coloring-array=Polynomial -b)
f3d_test(NAME TestInvalidUpDirection DATA suzanne.ply ARGS -g --up=W REGEXP "Could not set 'up'" NO_BASELINE)
f3d_test(NAME TestUpDirectionNoSign DATA suzanne.ply ARGS --up=X)
f3d_test(NAME TestTextureMatCap DATA suzanne.ply ARGS --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png)
f3d_test(NAME TestTexturesTransform DATA world.obj ARGS --textures-transform=1,0,0,0,-1,0,0,0,1)
f3d_test(NAME TestTexturesTransformGL DATA WaterBottle.glb ARGS --textures-transform=1,0,0,0,-1,0,0,0,1 --camera-direction=-1,0,0)
f3d_test(NAME TestConfigOrder DATA suzanne.ply ARGS CONFIG ${F3D_SOURCE_DIR}/testing/configs/config_order.json) # `.+` > `.*` alphabetically but overridden by the order
f3d_test(NAME TestOutputStream DATA suzanne.ply ARGS --verbose=quiet --output=- REGEXP ".PNG" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestOutputStreamInfo DATA suzanne.ply ARGS --verbose=info --output=- REGEXP "redirected to stderr" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestInvalidAntiAliasingMode DATA suzanne.ply ARGS --anti-aliasing=foo REGEXP "foo is an invalid antialiasing mode" NO_BASELINE)
f3d_test(NAME TestAntiAliasingModeDeprecated DATA suzanne.ply ARGS -a --anti-aliasing-mode=ssaa REGEXP "--anti-aliasing-mode is deprecated" NO_BASELINE)
f3d_test(NAME TestAntiAliasingDeprecated DATA suzanne.ply ARGS --anti-aliasing=true REGEXP "please specify the type of anti-aliasing" NO_BASELINE)
f3d_test(NAME TestTranslucencySupportDeprecated DATA suzanne.ply ARGS --translucency-support REGEXP "--translucency-support is deprecated" NO_BASELINE)
f3d_test(NAME TestInvalidPointSprites DATA suzanne.ply ARGS --point-sprites=foo REGEXP "foo is an invalid point sprites type" NO_BASELINE)
f3d_test(NAME TestPointSpritesTypeDeprecated DATA pointsCloud.vtp ARGS --point-sprites-type=sphere REGEXP "--point-sprites-type is deprecated" NO_BASELINE)
f3d_test(NAME TestPointSpritesDeprecated DATA pointsCloud.vtp ARGS --point-sprites=true REGEXP "please specify the type of point sprites" NO_BASELINE)
f3d_test(NAME TestInvalidBlendingMode DATA suzanne.ply ARGS --blending=foo REGEXP "foo is an invalid blending mode" NO_BASELINE)
f3d_test(NAME TestUserMatrixColoring DATA InterpolationTest.glb ARGS --scalar-coloring --coloring-array=TEXCOORD_0)
f3d_test(NAME TestSkinColoring DATA SimpleSkin.gltf ARGS --scalar-coloring --coloring-array=WEIGHTS_0)
f3d_test(NAME TestMorphColoring DATA SimpleMorph.gltf ARGS --scalar-coloring)
f3d_test(NAME TestAnimationUserMatrixColoring DATA InterpolationTest.glb ARGS --scalar-coloring --coloring-array=TEXCOORD_0 --animation-time=0.5)
f3d_test(NAME TestAnimationSkinColoring DATA SimpleSkin.gltf ARGS --scalar-coloring --coloring-array=WEIGHTS_0 --animation-time=2)
f3d_test(NAME TestAnimationMorphColoring DATA SimpleMorph.gltf ARGS --scalar-coloring --animation-time=0.5)
f3d_test(NAME TestAnimationInputChangeColoring DATA v_rock2.mdl ARGS --scalar-coloring --animation-time=0.01 --animation-indices=1)
f3d_test(NAME TestAnimationUserMatrixPointSprites DATA BoxAnimated.gltf ARGS --point-sprites --point-sprites-size=50 --animation-time=2)
f3d_test(NAME TestCollinearVectors DATA dragon.vtu ARGS --up=0,1,0 --camera-direction=0,2,0)
f3d_test(NAME TestAnimationInputChangePointSprites DATA v_rock2.mdl ARGS --point-sprites --point-sprites-size=50 --animation-time=0.01 --animation-indices=1)
f3d_test(NAME TestBackdropOpacityOpaque DATA suzanne.ply ARGS -n --backdrop-opacity=1.0 UI)
f3d_test(NAME TestBackdropOpacityTransparent DATA suzanne.ply ARGS -n --backdrop-opacity=0.0 UI)
f3d_test(NAME TestBackdropOpacityMedium DATA suzanne.ply ARGS -n --backdrop-opacity=0.5 UI)
f3d_test(NAME TestCustomOptionsNone DATA red_translucent_monkey.gltf ARGS --blending=none --anti-aliasing=none --point-sprites=none)

# Color map files testing
f3d_test(NAME TestColorMapFileFullPath DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/viridis8.png --scalar-coloring --coloring-component=1)
f3d_test(NAME TestColorMapInvalid DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/invalid.png --scalar-coloring REGEXP "Cannot read colormap at" NO_BASELINE)
f3d_test(NAME TestColorMapNonExistent DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/non_existent.png --scalar-coloring REGEXP "Cannot find the colormap" NO_BASELINE)
f3d_test(NAME TestColorMapGrayscale DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/white_grayscale.png --scalar-coloring REGEXP "The specified color map must have at least 3 channels" NO_BASELINE)
f3d_test(NAME TestColorMapMore1pxWarning DATA dragon.vtu ARGS --verbose=warning --colormap-file=${F3D_SOURCE_DIR}/testing/data/16bit.png --scalar-coloring REGEXP "The specified color map height is not equal to 1" NO_BASELINE)
f3d_test(NAME TestColorMap16bits DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/viridis16.png --scalar-coloring --coloring-component=1)
f3d_test(NAME TestColorMap32bits DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/viridis32.hdr --scalar-coloring --coloring-component=1)
f3d_test(NAME TestColorDiscretization DATA dragon.vtu ARGS --colormap-file=${F3D_SOURCE_DIR}/testing/data/viridis32.hdr --scalar-coloring --coloring-component=1 --colormap-discretization=4)

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/resources/colormaps/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/colormaps")
  f3d_test(NAME TestColorMapStem DATA dragon.vtu ARGS --colormap-file=magma --scalar-coloring --coloring-component=1)
  f3d_test(NAME TestColorMapFile DATA dragon.vtu ARGS --colormap-file=magma.png --scalar-coloring --coloring-component=1)
endif()

# Screenshot Interaction
function(f3d_ss_test)
  cmake_parse_arguments(F3D_SS_TEST "MINIMAL" "NAME;TEMPLATE;EXPECTED;DEPENDS" "ARGS" ${ARGN})
  if(NOT F3D_SS_TEST_MINIMAL)
    f3d_test(NAME TestScreenshot${F3D_SS_TEST_NAME} DATA suzanne.ply ARGS --screenshot-filename=${F3D_SS_TEST_TEMPLATE} --no-config --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log NO_BASELINE DEPENDS TestSetupScreenshots)
    f3d_test(NAME TestScreenshot${F3D_SS_TEST_NAME}File DATA suzanne.ply ARGS --reference=${F3D_SS_TEST_EXPECTED} DEPENDS TestScreenshot${F3D_SS_TEST_NAME} ${F3D_SS_TEST_DEPENDS} NO_BASELINE)
  else()
    # show filename, axes, fps before the "minimal screenshot" interaction; compare with --no-background only
    f3d_test(NAME TestScreenshot${F3D_SS_TEST_NAME} DATA suzanne.ply ARGS --screenshot-filename=${F3D_SS_TEST_TEMPLATE} --no-config -nxz --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshotMinimal.log NO_BASELINE DEPENDS TestSetupScreenshots)
    f3d_test(NAME TestScreenshot${F3D_SS_TEST_NAME}File DATA suzanne.ply ARGS --no-background --reference=${F3D_SS_TEST_EXPECTED} DEPENDS TestScreenshot${F3D_SS_TEST_NAME} ${F3D_SS_TEST_DEPENDS} NO_BASELINE)
  endif()
endfunction()
function(f3d_ss_template_test)
  cmake_parse_arguments(F3D_SS_TEMPLATE_TEST "" "NAME;TEMPLATE;EXPECTED_REGEX" "ARGS" ${ARGN})
  f3d_test(NAME TestScreenshot${F3D_SS_TEMPLATE_TEST_NAME} DATA suzanne.ply ARGS --screenshot-filename=${F3D_SS_TEMPLATE_TEST_TEMPLATE} --no-config --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log
    REGEXP "saving screenshot to .+[/\\]${F3D_SS_TEMPLATE_TEST_EXPECTED_REGEX}" NO_BASELINE DEPENDS TestSetupScreenshots)
endfunction()

cmake_path(SET _screenshot_path ${CMAKE_BINARY_DIR}/Testing/Temporary/ss)
cmake_path(SET _screenshot_user_path ${_screenshot_path}/user)
cmake_path(NATIVE_PATH _screenshot_path _screenshot_dir)
cmake_path(NATIVE_PATH _screenshot_user_path _screenshot_user_dir)

add_test(NAME f3d::TestClearScreenshots COMMAND ${CMAKE_COMMAND} -E remove_directory "${_screenshot_dir}")
set_tests_properties(f3d::TestClearScreenshots PROPERTIES FIXTURES_SETUP f3d::TestClearScreenshots_FIXTURE)

if(WIN32)
  cmake_path(SET _screenshot_windows_pictures_dir "${_screenshot_user_dir}/Pictures")
  cmake_path(NATIVE_PATH _screenshot_windows_pictures_dir _screenshot_windows_pictures_dir)
  # AppData/Local is required because on Windows changing USERPROFILE also impacts the cache location
  add_test(NAME f3d::TestSetupScreenshots COMMAND ${CMAKE_COMMAND} -E make_directory "${_screenshot_windows_pictures_dir}" -E make_directory "${_screenshot_user_dir}/AppData/Local")
else()
  add_test(NAME f3d::TestSetupScreenshots COMMAND ${CMAKE_COMMAND} -E make_directory "${_screenshot_user_dir}")
endif()
set_tests_properties(f3d::TestSetupScreenshots PROPERTIES FIXTURES_REQUIRED f3d::TestClearScreenshots_FIXTURE)

f3d_ss_test(NAME Version TEMPLATE ${_screenshot_dir}/{app}_{version}_{version_full}.png EXPECTED ${_screenshot_dir}/${PROJECT_NAME}_${F3D_VERSION}_${F3D_VERSION_FULL}.png)
f3d_ss_test(NAME Model TEMPLATE ${_screenshot_dir}/{model}_{model.ext}_{model_ext}.png EXPECTED ${_screenshot_dir}/suzanne_suzanne.ply_ply.png)
f3d_ss_test(NAME ModelN1 TEMPLATE ${_screenshot_dir}/{model}_{n}_{n:2}.png EXPECTED ${_screenshot_dir}/suzanne_1_01.png)
f3d_ss_test(NAME ModelN2 TEMPLATE ${_screenshot_dir}/{model}_{n}_{n:2}.png EXPECTED ${_screenshot_dir}/suzanne_2_02.png DEPENDS TestScreenshotModelN1)
f3d_ss_template_test(NAME Date TEMPLATE ${_screenshot_dir}/{model}_{date}_{date:%Y}.png EXPECTED_REGEX suzanne_[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]_[0-9][0-9][0-9][0-9]\.png)
f3d_ss_template_test(NAME InvalidFormats TEMPLATE ${_screenshot_dir}/{model}_{date:blah}_{n:blah}_{unknown}.png EXPECTED_REGEX suzanne_blah_1_\{unknown\}\.png)
f3d_ss_test(NAME Esc TEMPLATE ${_screenshot_dir}/{model}_{{model}}_{}.png EXPECTED ${_screenshot_dir}/suzanne_{model}_{}.png)
f3d_ss_test(NAME Minimal MINIMAL TEMPLATE ${_screenshot_dir}/minimal.png EXPECTED ${_screenshot_dir}/minimal.png)

if(WIN32)
f3d_ss_test(NAME UserModelN TEMPLATE {model}_{n}.png EXPECTED ${_screenshot_windows_pictures_dir}/suzanne_1.png)
else()
f3d_ss_test(NAME UserModelN TEMPLATE {model}_{n}.png EXPECTED ${_screenshot_user_dir}/suzanne_1.png)
endif()
set_tests_properties(f3d::TestScreenshotUserModelN PROPERTIES ENVIRONMENT "XDG_PICTURES_DIR=${_screenshot_user_dir};HOME=${_screenshot_user_dir};USERPROFILE=${_screenshot_user_dir}")

if(NOT APPLE OR VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
  f3d_test(NAME TestTextureColor DATA WaterBottle.glb ARGS --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo_mod.png --blending)
endif()

# Needs SSBO: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231108)
  if(APPLE) # MacOS does not support OpenGL 4.3
    f3d_test(NAME TestSkinningManyBonesFailure DATA tube_254bones.glb ARGS --verbose REGEXP "which requires OpenGL" NO_BASELINE)
  else()
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20241219) # The baseline changed with armature support
      # Strictly speaking, this test can also fail if ran without OpenGL 4.3 support on Windows and Linux
      # Instead of checking MacOS only, we should try to get OpenGL capabilities from CMake later instead
      f3d_test(NAME TestSkinningManyBones DATA tube_254bones.glb)
    endif()
  endif()
else()
  f3d_test(NAME TestSkinningManyBonesWarning DATA tube_254bones.glb ARGS --verbose REGEXP "with more than 250 bones \\\(254\\\)" NO_BASELINE)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestDefaultConfigFileHDRIFilename DATA dragon.vtu CONFIG config_build ARGS -j --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr UI LONG_TIMEOUT)
endif()

# Needs splat sorting with compute shaders
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  if(NOT APPLE) # MacOS does not support compute shaders
    f3d_test(NAME Test3DGaussiansSplatting DATA small.splat ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,-4.2)
    # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
      f3d_test(NAME TestDefaultConfigFileSPLAT DATA small.splat CONFIG config_build LONG_TIMEOUT UI)
    endif()
    f3d_test(NAME TestThumbnailConfigFileSPLAT DATA small.splat CONFIG thumbnail_build LONG_TIMEOUT)

    f3d_test(NAME Test3DGSPLY DATA bonsai_small.ply ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-2.6,0.5,-3.2)
    f3d_test(NAME Test3DGSPLYHDRI DATA bonsai_small.ply HDRI shanghai_bund_1k.hdr ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-2.6,0.5,-3.2)
    f3d_test(NAME TestInteractionPointSpritesCycle DATA bonsai_small.ply ARGS -sy --point-sprites-absolute-size --up=-Y --blending=sort --camera-position=-2.6,0.5,-3.2 INTERACTION) #OOOOO

    f3d_test(NAME TestSPZDegree0 DATA hornedlizard_small_d0.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
    f3d_test(NAME TestInvalidSH DATA invalidSH.vtp ARGS -osy --verbose REGEXP "Spherical harmonics array is not valid" NO_BASELINE)

    # Needs texture array support: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12112
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250513)
      f3d_test(NAME TestSPZDegree1 DATA hornedlizard_small_d1.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree2 DATA hornedlizard_small_d2.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree3 DATA hornedlizard_small_d3.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree1Stochastic DATA hornedlizard_small_d1.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=stochastic --camera-position=-3.6,0.5,4.2) # Test instancing with spherical harmonics

      # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
      if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
        f3d_test(NAME TestDefaultConfigFileSPZ DATA hornedlizard_small_d3.spz CONFIG config_build LONG_TIMEOUT UI)
      endif()
      f3d_test(NAME TestThumbnailConfigFileSPZ DATA hornedlizard_small_d3.spz CONFIG thumbnail_build LONG_TIMEOUT)
      f3d_test(NAME TestSPZVersion3 DATA bonsai.spz ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-2.6,0.5,-3.2)
      f3d_test(NAME TestSPZCorruptedGZIP DATA wrong_spz_gzip_corrupted.spz ARGS -osy --verbose REGEXP "Invalid GZIP file" NO_BASELINE)
      f3d_test(NAME TestSPZTruncatedGZIP DATA truncated_gzip.spz ARGS -osy --verbose REGEXP "Invalid GZIP file" NO_BASELINE)
      f3d_test(NAME TestWrongSPZMagic DATA wrong_spz_magic.spz ARGS -osy --verbose REGEXP "Incompatible SPZ header" NO_BASELINE)
      f3d_test(NAME TestWrongSPZVersion DATA wrong_spz_version.spz ARGS -osy --verbose REGEXP "Incompatible SPZ version. Only 2 and 3 are supported" NO_BASELINE)
    endif()
  endif()
endif()

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10662
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231102)
  f3d_test(NAME Test3DGaussiansSplattingStochastic DATA small.splat ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=stochastic --camera-position=-3.6,0.5,-4.2)
endif()

# Test plugin fail code path
f3d_test(NAME TestPluginVerbose ARGS --verbose REGEXP "Loading plugin \"native\"" NO_BASELINE)
f3d_test(NAME TestPluginNonExistent ARGS --load-plugins=dummy REGEXP "Plugin failed to load" NO_BASELINE)
if(WIN32)
  set(_TEST_PLUGIN_INVALID_REGEXP "is not a valid Win32 application")
elseif(APPLE)
  set(_TEST_PLUGIN_INVALID_REGEXP "not.*mach-o file")
else()
  set(_TEST_PLUGIN_INVALID_REGEXP "file too short")
endif()
f3d_test(NAME TestPluginInvalid ARGS --load-plugins=${F3D_SOURCE_DIR}/testing/data/invalid.so REGEXP ${_TEST_PLUGIN_INVALID_REGEXP} NO_BASELINE)

if(UNIX AND NOT APPLE)
  f3d_test(NAME TestPluginInvalidSystem ARGS --verbose --load-plugins=invalid REGEXP "file too short" ENV "LD_LIBRARY_PATH=${F3D_SOURCE_DIR}/testing/data" NO_BASELINE)
endif()

if(WIN32)
  set(_dirname "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
  set(_dirname "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif()

if(BUILD_SHARED_LIBS)
  f3d_test(NAME TestPluginNoInit ARGS --verbose --load-plugins=${_dirname}/${CMAKE_SHARED_LIBRARY_PREFIX}f3d${CMAKE_SHARED_LIBRARY_SUFFIX} NO_BASELINE REGEXP "Cannot find init_plugin symbol in library")
endif()

if(NOT F3D_MACOS_BUNDLE)
  # On linux, we can easily test the config file search from the binary code by positioning a config file in the binary dir
  configure_file("${F3D_SOURCE_DIR}/testing/configs/complex.json" "${CMAKE_BINARY_DIR}/share/f3d/configs/complex_build.json" COPYONLY)
  file(COPY "${F3D_SOURCE_DIR}/resources/configs/config.d/" "${F3D_SOURCE_DIR}/plugins/native/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestConfigFileBuild DATA dragon.vtu CONFIG complex_build.json UI)
    f3d_test(NAME TestConfigStemBuild DATA dragon.vtu CONFIG complex_build UI)
    f3d_test(NAME TestConfigFileUpperCase DATA suzanne_upper.STL CONFIG complex_build UI)
    f3d_test(NAME TestConfigFileMultiFileSTL DATA mb/recursive/mb_1_0.vtp suzanne.stl ARGS --multi-file-mode=all CONFIG complex_build UI)
    f3d_test(NAME TestConfigFileMultiFileVTP DATA mb/recursive/mb_1_0.vtp suzanne.stl mb/recursive/mb_2_0.vtp ARGS --multi-file-mode=all CONFIG complex_build UI)

    f3d_test(NAME TestDefaultConfigFileVTU DATA dragon.vtu CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileVTI DATA vase_4comp.vti CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileSTL DATA suzanne.stl CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileTIFF DATA f3d.tif CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI) # Note: This tests config file order as camera_direction is set in different files
    f3d_test(NAME TestDefaultConfigFilePLY DATA suzanneRGBA.ply CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileQuakeMDL DATA zombie.mdl CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAndCommand DATA suzanne.stl ARGS --up=-Y --camera-direction=-1,0.5,-1 CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigTranslucent DATA red_translucent_monkey.gltf CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigRemoveEmptyFileGroups DATA invalid.vtp cow.vtp CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()

  file(COPY "${F3D_SOURCE_DIR}/resources/configs/thumbnail.d/" "${F3D_SOURCE_DIR}/plugins/native/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileVTU DATA dragon.vtu CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileVTI DATA vase_4comp.vti CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileSTL DATA suzanne.stl CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFilePLY DATA suzanneRGBA.ply CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileQuakeMDL DATA zombie.mdl CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
endif()

f3d_test(NAME TestTextureColorWithOptions DATA WaterBottle.glb ARGS --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo_mod.png --color=1,1,0 --opacity=0.4 --blending)
f3d_test(NAME TestGLTFImporterUnlit DATA UnlitTest.glb)
f3d_test(NAME TestMaterial DATA suzanne.ply ARGS --color=0.72,0.45,0.2 --metallic=0.7 --roughness=0.2)
f3d_test(NAME TestMaterialFullScene DATA WaterBottle.glb ARGS --color=0.9,0.1,0.1 --metallic=0.7 --roughness=0.2)
f3d_test(NAME TestMinBaseIOR DATA suzanne.ply ARGS --base-ior=1.0)
f3d_test(NAME TestMaxBaseIOR DATA suzanne.ply ARGS --base-ior=2.5)
f3d_test(NAME TestMetaData DATA pdiag.vtu ARGS -m UI)
f3d_test(NAME TestEdges DATA suzanne.ply ARGS -e)
f3d_test(NAME TestLineWidth DATA cow.vtk ARGS -e --line-width=5)
f3d_test(NAME TestLineWidthFullScene DATA suzanne.obj ARGS -e --line-width=3 --up=-Y)
f3d_test(NAME TestTextureMatCapWithEdges DATA suzanne.ply ARGS -e --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png)

# Test enabling all animations
f3d_test(NAME TestAnimationAllAnimations DATA InterpolationTest.glb ARGS --animation-indices=-1 --animation-time=1 --animation-progress)

# Test enabling no animations
f3d_test(NAME TestAnimationNoAnimations DATA InterpolationTest.glb ARGS --animation-indices= --verbose NO_BASELINE REGEXP "Current animation is: No animation")

# Test Verbose animation, no baseline needed
f3d_test(NAME TestVerboseAnimation DATA InterpolationTest.glb ARGS --verbose NO_BASELINE REGEXP "7: CubicSpline Translation")

# Test interactive animation and speed factor
f3d_test(NAME TestInteractionAnimation DATA f3d.glb ARGS --animation-progress INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationFast DATA f3d.glb ARGS --animation-progress --animation-speed-factor=1.5 INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationSlow DATA f3d.glb ARGS --animation-progress --animation-speed-factor=0.5 INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationFrameRate DATA f3d.glb ARGS --animation-progress --frame-rate=1 INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationCameraMovement DATA CameraAnimated.glb ARGS --camera-index=0 --animation-progress INTERACTION)#Space;MouseMovement;Space;
f3d_test(NAME TestInteractionAnimationInvert DATA f3d.glb ARGS --animation-speed-factor=-1 --animation-progress INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationBackward DATA f3d.glb ARGS --animation-progress INTERACTION)#Ctrl+Shift+Space;Wait;Space;
f3d_test(NAME TestCommandScriptJumpToPreviousFrame DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress SCRIPT TestCommandScriptJumpToPreviousFrame.txt)
f3d_test(NAME TestCommandScriptJumpToNextFrame DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress SCRIPT TestCommandScriptJumpToNextFrame.txt)
f3d_test(NAME TestCommandScriptJumpToFirstFrame DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress SCRIPT TestCommandScriptJumpToFirstFrame.txt)
f3d_test(NAME TestCommandScriptJumpToLastFrame DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress SCRIPT TestCommandScriptJumpToLastFrame.txt)
f3d_test(NAME TestCommandScriptJumpToMiddleFrame DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress SCRIPT TestCommandScriptJumpToMiddleFrame.txt)

# Test temporal anti-aliasing
f3d_test(NAME TestInteractionTAA DATA suzanne.ply ARGS --anti-aliasing=taa INTERACTION) #Render;Render...
f3d_test(NAME TestInteractionTAAMiddleClick DATA suzanne.ply ARGS --anti-aliasing=taa INTERACTION) #Render;Render...;MiddleClick;Render;Render...

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231102)
  f3d_test(NAME TestInteractionPointCloudTAA DATA pointsCloud.vtp ARGS --anti-aliasing=taa -o --point-sprites-size=20 INTERACTION) #Render;Render...
endif()

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12098
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250504)
  f3d_test(NAME TestInteractionStochasticTAA DATA alpha.glb ARGS --anti-aliasing=taa --blending=stochastic INTERACTION) #Render;Render...
  f3d_test(NAME TestInteractionAndCLIBlending ARGS --blending DATA suzanne.stl alpha.glb INTERACTION)#PP;Right
endif()

# Need SSIM comparison for some reason
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240729)
  # Test scrolling cheatsheet with no model by mouse wheel and by using scrollbar
  f3d_test(NAME TestInteractionNoModelScrollWheel RESOLUTION 1000,600 NO_DATA_FORCE_RENDER LONG_TIMEOUT INTERACTION UI)
  f3d_test(NAME TestInteractionNoModelScrollBar RESOLUTION 1000,600 NO_DATA_FORCE_RENDER LONG_TIMEOUT INTERACTION UI)
endif()

# https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12678
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251206)
  f3d_test(NAME TestPointCloudFullScene DATA pointsCloud.gltf ARGS --point-size=20)
endif()

# A verbose test that needs animation index support
f3d_test(NAME TestVerboseAnimationWrongAnimationTimeHigh DATA BoxAnimated.gltf ARGS --animation-time=10 --verbose REGEXP "Animation time 10 is outside of range \\[0, 3\\.70833\\], using 3\\.70833" NO_BASELINE)
f3d_test(NAME TestVerboseAnimationWrongAnimationTimeLow DATA BoxAnimated.gltf ARGS --animation-time=-5 --verbose REGEXP "Animation time -5 is outside of range \\[0, 3\\.70833\\], using 0" NO_BASELINE)

# Verbose test for animation
f3d_test(NAME TestVerboseAnimationTimeRange DATA InterpolationTest.glb ARGS --verbose REGEXP "0, 1.66667" NO_BASELINE)
f3d_test(NAME TestCommandScriptVerboseMultiAnimationTimeRange SCRIPT TestCommandScriptVerboseMultiAnimationTimeRange.txt DATA InterpolationTest.glb ARGS --verbose REGEXP "0, 1.70833" NO_BASELINE)# cycle_animation x3
f3d_test(NAME TestVerboseQuakeMDLAnimationNoNamingScheme ARGS --verbose DATA v_rock2.mdl REGEXP "0: flame" NO_BASELINE)
f3d_test(NAME TestVerboseQuakeMDLGroupSkin ARGS --verbose DATA groupskin.mdl REGEXP "0: group_skin" NO_BASELINE)
f3d_test(NAME TestVerboseQuakeMDLInvalid ARGS --verbose DATA w_medkit_hl.mdl REGEXP "Unsupported MDL version" NO_BASELINE)

# Armature test
f3d_test(NAME TestGLTFRigArmatureNoArmature DATA RiggedFigure.glb ARGS --animation-time=1 --opacity=0.5 -p)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20241219)
  f3d_test(NAME TestGLTFRigArmature DATA RiggedFigure.glb ARGS --animation-time=1 --armature)
  f3d_test(NAME TestGLTFRigArmatureWithOpacity DATA RiggedFigure.glb ARGS --animation-time=1 --armature --opacity=0.5 -p)
  f3d_test(NAME TestGLTFRigArmatureSphereTube DATA RiggedFigure.glb ARGS --animation-time=1 --armature --point-size=20 --line-width=5)
endif()

# Test exit hotkey
f3d_test(NAME TestInteractionSimpleExit DATA cow.vtp REGEXP "Interactor has been stopped" INTERACTION NO_BASELINE) #CTRL+Q

# Cheatsheet test with minimal resolution to avoid repeated change of baselines
f3d_test(NAME TestInteractionCheatsheetWhiteBG DATA cow.vtp ARGS --background-color=1,1,1 INTERACTION UI) #H
f3d_test(NAME TestInteractionCheatsheetBlackBG DATA cow.vtp ARGS --background-color=0,0,0 INTERACTION UI) #H
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionCheatsheetNoFile INTERACTION UI NO_DATA_FORCE_RENDER) #HXM
endif()
f3d_test(NAME TestInteractionCheatsheetScalars DATA dragon.vtu ARGS --scalar-coloring --coloring-component=-2 INTERACTION UI LONG_TIMEOUT) #HSSS
f3d_test(NAME TestInteractionCheatsheetScalarsNoArray DATA dragon.vtu ARGS --scalar-coloring INTERACTION UI RESOLUTION 500,300) #H
f3d_test(NAME TestInteractionCheatsheetOpacity DATA cow.vtp INTERACTION UI ARGS --opacity=0.5 RESOLUTION 300,700) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetAnimationName DATA InterpolationTest.glb ARGS --animation-indices=6 INTERACTION UI) #HWWW
f3d_test(NAME TestInteractionCheatsheetConfigFile DATA dragon.vtu CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetMultiModifierBinding DATA dragon.vtu RESOLUTION 1200,300 CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetCycle DATA cow.vtp RESOLUTION 800,300 INTERACTION UI LONG_TIMEOUT) #HAAPO

if(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestInteractionCheatsheetCentered DATA cow.vtp RESOLUTION 500,1500 INTERACTION UI LONG_TIMEOUT) #H
endif()

f3d_test(NAME TestCameraPersp DATA Cameras.gltf ARGS --camera-index=0)
f3d_test(NAME TestCameraOrtho DATA Cameras.gltf ARGS --camera-index=1)
f3d_test(NAME TestCameraIndexConfiguration DATA Cameras.gltf ARGS --camera-index=0  --camera-azimuth-angle=15 --camera-position=0.7,0.5,3)
f3d_test(NAME TestCameraIndexInvalid DATA Cameras.gltf ARGS --camera-index=3 REGEXP "is higher than the number of available camera" NO_BASELINE)
f3d_test(NAME TestCameraIndexNegative DATA Cameras.gltf ARGS --camera-index=-1 REGEXP "Invalid camera index" NO_BASELINE)

# Require improved importer support https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11303
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240910)
  f3d_test(NAME TestInvalidFileFileNameEmpty DATA invalid.vtp ARGS --filename NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileInvalid DATA cow.vtp invalid.vtp dragon.vtu ARGS --multi-file-mode=all NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileUnsupportedFilesFileName DATA unsupportedFile.dummy cow.vtp ARGS --multi-file-mode=all --filename NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileCameraIndex DATA Cameras.gltf CameraAnimated.glb ARGS --multi-file-mode=all --camera-index=2 --opacity=0.5 --blending)
  f3d_test(NAME TestConsoleBadgeError DATA invalid.vtp NO_DATA_FORCE_RENDER UI)
endif()

# Test Verbose camera
f3d_test(NAME TestVerboseCamera DATA Cameras.gltf ARGS --camera-index=1 --verbose NO_RENDER REGEXP "0:.*1:")

f3d_test(NAME TestGLTFMorph DATA SimpleMorph.gltf)
f3d_test(NAME TestGLTFURI DATA Lantern/Lantern.gltf)

f3d_test(NAME TestNoBackground DATA cow.vtp ARGS --no-background)

# HDRI tests
f3d_test(NAME TestHDRI DATA suzanne.ply HDRI palermo_park_1k.hdr)
f3d_test(NAME TestHDRICache DATA suzanne.ply HDRI palermo_park_1k.hdr DEPENDS TestHDRI)
f3d_test(NAME TestHDRIBlur DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -u)
f3d_test(NAME TestHDRIBlurCoCSmall DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=10 --camera-position=-20,0,20)
f3d_test(NAME TestHDRIBlurCoCMedium DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=50 --camera-position=-20,0,20)
f3d_test(NAME TestHDRIBlurCoCLarge DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=100 --camera-position=-20,0,20)
f3d_test(NAME TestHDRIBlurCoCZero DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=0 --camera-position=-20,0,20 THRESHOLD 0.08) # Threshold is needed for legacy comparison method for VTK 9.3
f3d_test(NAME TestHDRIBlurCoCNegative DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=-100 --camera-position=-20,0,20)
f3d_test(NAME TestHDRIBlurRatio DATA suzanne.ply HDRI palermo_park_1k.hdr RESOLUTION 600,100 ARGS -u)
f3d_test(NAME TestHDRIEdges DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -e THRESHOLD 0.06)
f3d_test(NAME TestHDRI8Bit DATA suzanne.ply HDRI f3d.tif ARGS --color=1.0,0.0,0.0)
f3d_test(NAME TestHDRIOrient DATA suzanne.stl HDRI palermo_park_1k.hdr ARGS --up=+Z)
f3d_test(NAME TestHDRIToneMapping DATA suzanne.ply HDRI palermo_park_1k.hdr TONE_MAPPING ARGS -t)
f3d_test(NAME TestInteractionHDRIMove DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION THRESHOLD 0.05) #Shift+MouseRight; # Threshold is needed for legacy image comparison for VTK 9.3
f3d_test(NAME TestInteractionHDRIBlur DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION) #U
f3d_test(NAME TestInteractionHDRIReload DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION) #Up
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionHDRIChange DATA multi HDRI palermo_park_1k.hdr CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #Left
endif()
# Test non existent HDRI, do not add a dummy.png
f3d_test(NAME TestNonExistentHDRI DATA cow.vtp HDRI dummy.png REGEXP "HDRI file does not exist" NO_BASELINE)

# Test invalid provided HDRI
f3d_test(NAME TestInvalidHDRI DATA cow.vtp HDRI invalid.png REGEXP "Cannot open HDRI file" NO_BASELINE)

# Use a dummy HDRI for simplicity to test default HDRI
f3d_test(NAME TestHDRIDefault DATA suzanne.ply HDRI dummy.png THRESHOLD 0.11) # Threshold is needed for legacy comparison method for VTK 9.3

configure_file("${F3D_SOURCE_DIR}/testing/configs/hdri.json.in" "${CMAKE_BINARY_DIR}/hdri.json")
f3d_test(NAME TestConfigFileHDRI DATA dragon.vtu CONFIG "${CMAKE_BINARY_DIR}/hdri.json" LONG_TIMEOUT)

if(F3D_MODULE_EXR)
  f3d_test(NAME TestHDRIEXR DATA suzanne.ply HDRI kloofendal_43d_clear_1k.exr)
endif()

f3d_test(NAME TestHDRISkyboxOnly DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-skybox LONG_TIMEOUT)
f3d_test(NAME TestHDRIAmbientOnly DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-ambient LONG_TIMEOUT)
f3d_test(NAME TestHDRIAmbientOnlyNoBackground DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-ambient --no-background LONG_TIMEOUT)
f3d_test(NAME TestHDRINone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr LONG_TIMEOUT)

f3d_test(NAME TestInteractionHDRICache DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION DEPENDS TestHDRI) #FFFFJJJJ
f3d_test(NAME TestInteractionHDRIRemoveSkybox DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRIRemoveAmbient DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRIRemoveBoth DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRILoop DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRIFullFromNone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr LONG_TIMEOUT INTERACTION)

if(F3D_MODULE_RAYTRACING)
  # https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10753
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231213)
    # XXX: These tests are impacted by https://github.com/f3d-app/f3d/issues/933
    f3d_test(NAME TestHDRIRaytracing DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4)
    f3d_test(NAME TestHDRIRaytracingSkyboxOnly DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-skybox -rd --raytracing-samples=4 LONG_TIMEOUT)
    f3d_test(NAME TestHDRIRaytracingAmbientOnly DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-ambient -rd --raytracing-samples=4 LONG_TIMEOUT)
    f3d_test(NAME TestHDRIRaytracingAmbientOnlyNoBackground DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-ambient -rd --raytracing-samples=4 --no-background LONG_TIMEOUT)
    f3d_test(NAME TestHDRIRaytracingNone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr -rd --raytracing-samples=4 LONG_TIMEOUT)

    f3d_test(NAME TestInteractionHDRIRaytracingRemoveSkybox DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingRemoveAmbient DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingRemoveBoth DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingLoop DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingFullFromNone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr -rd --raytracing-samples=4 INTERACTION LONG_TIMEOUT)
  endif()
endif()

# SSAA with HDR framebuffer support in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12026
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250329)
  f3d_test(NAME TestHDRIToneMappingSSAA DATA suzanne.ply HDRI palermo_park_1k.hdr TONE_MAPPING ARGS -t --anti-aliasing=ssaa)
endif()

# TGA support for OBJ added in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11922
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250220)
  f3d_test(NAME TestOBJWithTGATexture DATA world_tga.obj)
endif()

if(F3D_MODULE_RAYTRACING)
  # Ideally we could test these with ospray 2.7 and VTK 9.3
  # https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10753
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231213)
    f3d_test(NAME TestRaytracingGLTF DATA WaterBottle.glb ARGS -rd --raytracing-samples=4)
    f3d_test(NAME TestRaytracingBackground DATA suzanne.ply ARGS -rd --raytracing-samples=4 --background-color=1,0,0 THRESHOLD 0.05) # Threshold needed because of difference in rendering in VTK 9.3
    f3d_test(NAME TestRaytracingPointCloud DATA pointsCloud.vtp ARGS -rd --raytracing-samples=4 --point-size=20 THRESHOLD 0.1) # Threshold needed because of difference in rendering in VTK 9.3
    f3d_test(NAME TestRaytracingDenoise DATA suzanne.ply ARGS -rd --raytracing-samples=4)
    f3d_test(NAME TestRaytracingNoDenoise DATA suzanne.ply ARGS -r --raytracing-samples=20)
    f3d_test(NAME TestVersionRaytracing ARGS --version REGEXP "Module Raytracing: ON")
    f3d_test(NAME TestInteractionRaytracingDenoise DATA suzanne.ply ARGS --raytracing-samples=4 INTERACTION) #RD
    f3d_test(NAME TestRaytracingScalarBar DATA dragon.vtu ARGS -rsbd --raytracing-samples=4 THRESHOLD 0.06) # Threshold needed because of difference in rendering in VTK 9.3 on macOS

    if(NOT F3D_MACOS_BUNDLE)
      # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
      if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
        f3d_test(NAME TestRaytracingDefaultConfigFile DATA dragon.vtu CONFIG config_build ARGS -rd --raytracing-samples=4 LONG_TIMEOUT TONE_MAPPING)
      endif()

      f3d_test(NAME TestRaytracingThumbnailConfigFile DATA dragon.vtu CONFIG thumbnail_build ARGS -rd --raytracing-samples=4 LONG_TIMEOUT TONE_MAPPING)
    endif()

    f3d_test(NAME TestRaytracingNoBackground DATA suzanne.ply ARGS -rd --raytracing-samples=4 --no-background)
  endif()
else(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestCommandScriptRaytracingNoRaytracing DATA suzanne.ply SCRIPT TestCommandScriptRaytracing.txt NO_BASELINE REGEXP "Raytracing options can't be used if F3D has not been built with raytracing")
endif()

if(F3D_MODULE_EXR)
  f3d_test(NAME TestVersionEXR ARGS --version REGEXP "Module OpenEXR: ON")
endif()

if(F3D_MODULE_WEBP)
  f3d_test(NAME TestVersionWebP ARGS --version REGEXP "Module WebP: ON")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11922
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250220)
    f3d_test(NAME TestOBJWebP DATA cube_webp.obj)
  endif()
endif()

if(F3D_PLUGIN_BUILD_ALEMBIC)
  include(tests.alembic.cmake)
endif()

if(F3D_PLUGIN_BUILD_ASSIMP)
  include(tests.assimp.cmake)
endif()

if(F3D_PLUGIN_BUILD_DRACO)
  include(tests.draco.cmake)
endif()

if(F3D_PLUGIN_BUILD_HDF)
  include(tests.hdf.cmake)
endif()

if(F3D_PLUGIN_BUILD_WEBIFC)
  include(tests.webifc.cmake)
endif()

if(F3D_PLUGIN_BUILD_OCCT)
  include(tests.occt.cmake)
endif()

if(F3D_PLUGIN_BUILD_USD)
  include(tests.usd.cmake)
endif()

if(F3D_PLUGIN_BUILD_VDB)
  include(tests.vdb.cmake)
endif()

include(tests.watch.cmake)
