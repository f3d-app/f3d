# F3D Testing
function(f3d_test)

	cmake_parse_arguments(F3D_TEST "TONE_MAPPING;LONG_TIMEOUT;INTERACTION;INTERACTION_CONFIGURE;NO_BASELINE;NO_RENDER;NO_OUTPUT;WILL_FAIL;NO_DATA_FORCE_RENDER;UI" "NAME;CONFIG;RESOLUTION;THRESHOLD;REGEXP;REGEXP_FAIL;HDRI;SCRIPT;RENDERING_BACKEND" "DATA;DEPENDS;ENV;ARGS" ${ARGN})

  if(F3D_TEST_CONFIG)
    list(APPEND F3D_TEST_ARGS "--config=${F3D_TEST_CONFIG}")
  else()
    list(APPEND F3D_TEST_ARGS "--no-config")
  endif()

  if (F3D_TEST_DATA)
    foreach(_single_data ${F3D_TEST_DATA})
      list(APPEND F3D_TEST_ARGS "${F3D_SOURCE_DIR}/testing/data/${_single_data}")
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

  # make tests w/ scalar-coloring sensitive to increase in max colors
  set(_has_scalar_coloring FALSE)  
  if(F3D_TEST_CONFIG) # checks for tests w/ complex config (sets scalar coloring)
    if("${F3D_TEST_CONFIG}" MATCHES "complex")
      set(_has_scalar_coloring TRUE) 
    endif()
  endif()
  if(F3D_TEST_ARGS)   # checks for tests w/ scalar-coloring in args
    foreach(_arg ${F3D_TEST_ARGS})
      if(_arg STREQUAL "-s" OR _arg STREQUAL "--scalar-coloring")
        set(_has_scalar_coloring TRUE)
      elseif(_arg MATCHES "--reference-threshold=") # if threshold is set for whatever reason, don't override
        set(_has_scalar_coloring FALSE)
        break()
      endif()
    endforeach()
  endif()
  if(_has_scalar_coloring)
    list(APPEND F3D_TEST_ARGS "--reference-threshold=0.0")
  endif()

  add_test(NAME "f3d::${F3D_TEST_NAME}" COMMAND $<TARGET_FILE:f3d> ${F3D_TEST_ARGS} COMMAND_EXPAND_LISTS)

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
f3d_test(NAME TestSPLAT DATA small.splat ARGS -osy --up=-Y --point-sprites-size=1)
f3d_test(NAME TestQuakeMDL DATA zombie.mdl)
f3d_test(NAME TestQuakeMDLAnimationSimpleFrame DATA zombie.mdl ARGS --animation-time=0.5)
f3d_test(NAME TestQuakeMDLAnimationBetween DATA zombie.mdl ARGS --animation-time=1.42)
f3d_test(NAME TestQuakeMDLAnimationMulti DATA soldier_animations.mdl ARGS --animation-index=2 --animation-time=0.5)
f3d_test(NAME TestQuakeMDLAnimationGroupFrame DATA flame_mixed.mdl ARGS --animation-index=1 --animation-time=0.5)
f3d_test(NAME TestQuakeMDLDisableAnimation DATA zombie.mdl soldier_animations.mdl ARGS --multi-file-mode=all --animation-time=0.5)
f3d_test(NAME TestQuakeMDLSkinIndex DATA armor.mdl ARGS -DQuakeMDL.skin_index=2)
f3d_test(NAME TestQuakeMDLSkinIndexNegative DATA armor.mdl ARGS -DQuakeMDL.skin_index=-1)
f3d_test(NAME TestQuakeMDLSkinIndexNonInteger DATA armor.mdl ARGS -DQuakeMDL.skin_index=1.5)
f3d_test(NAME TestQuakeMDLSkinIndexOutOfBounds DATA armor.mdl ARGS -DQuakeMDL.skin_index=4)
f3d_test(NAME TestQuakeMDLSkinIndexOverflow DATA armor.mdl ARGS -DQuakeMDL.skin_index=9223372036854775808)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_test(NAME TestQuakeMDLActorCollection DATA zombie.mdl ARGS --scalar-coloring)
endif()
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
f3d_test(NAME TestAxis DATA suzanne.ply ARGS -x)
f3d_test(NAME TestBackfaceVisible DATA backface.vtp ARGS --backface-type=visible)
f3d_test(NAME TestBackfaceHidden DATA backface.vtp ARGS --backface-type=hidden)
f3d_test(NAME TestPointCloud DATA pointsCloud.vtp ARGS -o --point-sprites-size=20)
f3d_test(NAME TestPointCloudBar DATA pointsCloud.vtp ARGS -sob --point-sprites-size=20)
f3d_test(NAME TestPointCloudUG DATA pointsCloud.vtu ARGS -o --point-sprites-size=20)
f3d_test(NAME TestPointCloudVolume DATA bluntfin.vts ARGS -sob)
f3d_test(NAME TestPointCloudDefaultScene DATA pointsCloud.vtp ARGS --point-size=20)
f3d_test(NAME Test3DSImporter DATA iflamigm.3ds ARGS --up=+Z)
f3d_test(NAME TestScalars DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1)
f3d_test(NAME TestScalarsCell DATA f3d.vtp ARGS --scalar-coloring --coloring-by-cells --coloring-component=-2 --up=+Z)
f3d_test(NAME TestScalarsRange DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1 --coloring-range=0,1)
f3d_test(NAME TestScalarsWithBar DATA suzanne.ply ARGS -b -s --coloring-array=Normals --coloring-component=0)
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
f3d_test(NAME TestFilenameWhiteBg DATA suzanne.ply ARGS -n --background-color=1,1,1 UI)
f3d_test(NAME TestConsoleBadgeWarning DATA suzanne.ply ARGS --position=0 UI)
f3d_test(NAME TestConsoleBadgeQuiet DATA suzanne.ply ARGS --position=0 --verbose=quiet UI)
f3d_test(NAME TestCityGML DATA Part-4-Buildings-V4-one.gml)
f3d_test(NAME TestPTS DATA samplePTS.pts)
f3d_test(NAME TestColormap DATA IM-0001-1983.dcm ARGS --scalar-coloring --roughness=1 --colormap=0,1,0,0,1,0,1,0)
f3d_test(NAME TestCameraConfiguration DATA suzanne.obj ARGS --camera-position=0,0,-10 -x --camera-view-up=+X --camera-focal-point=1,0,0 --camera-view-angle=20 --camera-azimuth-angle=40 --camera-elevation-angle=-80 --camera-direction=12,34,56 --camera-zoom-factor=78)
f3d_test(NAME TestCameraDirection DATA suzanne.obj ARGS --camera-direction=-xy+z)
f3d_test(NAME TestCameraClipping DATA checkerboard_colorful.obj CONFIG ${F3D_SOURCE_DIR}/testing/configs/checkerboard_colorful.json RESOLUTION 800,600)
f3d_test(NAME TestCameraOrthographic ARGS --camera-orthographic DATA cow.vtp)
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
f3d_test(NAME TestMultiFileMetaData DATA mb/recursive ARGS -m --multi-file-mode=all UI)
f3d_test(NAME TestMultiFileModeDir DATA mb ARGS -n --multi-file-mode=dir UI)
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
f3d_test(NAME TestConfigImperative DATA dragon.vtu ARGS --axis=false CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json)
f3d_test(NAME TestAnimationIndex DATA InterpolationTest.glb ARGS --animation-index=7 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestMultiFileAnimationIndex DATA InterpolationTest.glb BoxAnimated.gltf ARGS --animation-index=9 --animation-time=0.85 --animation-progress --multi-file-mode=all)
f3d_test(NAME TestMultiFileAnimationNoAnimationSupport DATA f3d.glb world.obj ARGS --multi-file-mode=all --animation-time=2 --animation-progress)
f3d_test(NAME TestAnimationAutoplay DATA InterpolationTest.glb ARGS --animation-autoplay)
f3d_test(NAME TestMaxSizeBelow DATA suzanne.stl ARGS --max-size=1)
f3d_test(NAME TestMaxSizeAbove DATA WaterBottle.glb ARGS --max-size=0.2 REGEXP "file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestMaxSizeAboveMultiFile DATA suzanne.obj WaterBottle.glb ARGS --multi-file-mode=all --max-size=0.6 --translucency-support --opacity=0.5)
f3d_test(NAME TestAlternativeOptionSyntax DATA WaterBottle.glb ARGS --max-size 0.2 REGEXP "file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestNonExistentFile DATA nonExistentFile.vtp ARGS --filename WILL_FAIL)
f3d_test(NAME TestUnsupportedFile DATA unsupportedFile.dummy ARGS --filename WILL_FAIL)
f3d_test(NAME TestComponentName DATA from_abq.vtu ARGS --scalar-coloring --coloring-scalar-bar --coloring-component=2)
f3d_test(NAME TestNoRender DATA dragon.vtu NO_RENDER)
f3d_test(NAME TestNoRenderWithOptions DATA dragon.vtu ARGS --hdri-ambient --axis NO_RENDER) # These options causes issues if not handled correctly
f3d_test(NAME TestNoFile NO_DATA_FORCE_RENDER UI)
f3d_test(NAME TestNoFileEmptyFileName ARGS --filename NO_DATA_FORCE_RENDER UI)
f3d_test(NAME TestMultiFile DATA mb/recursive ARGS --multi-file-mode=all)
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
f3d_test(NAME TestTextureMatCapWithTCoords DATA WaterBottle.glb ARGS --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png)
f3d_test(NAME TestConfigOrder DATA suzanne.ply ARGS CONFIG ${F3D_SOURCE_DIR}/testing/configs/config_order.json) # `.+` > `.*` alphabetically but overridden by the order
f3d_test(NAME TestOutputStream DATA suzanne.ply ARGS --verbose=quiet --output=- REGEXP "^.PNG" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestOutputStreamInfo DATA suzanne.ply ARGS --verbose=info --output=- REGEXP "redirected to stderr" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestInvalidAntiAliasingMode DATA suzanne.ply ARGS -a --anti-aliasing-mode=foo REGEXP "foo is an invalid antialiasing mode" NO_BASELINE)
f3d_test(NAME TestUserMatrixColoring DATA InterpolationTest.glb ARGS --scalar-coloring --coloring-array=TEXCOORD_0)
f3d_test(NAME TestSkinColoring DATA SimpleSkin.gltf ARGS --scalar-coloring --coloring-array=WEIGHTS_0)
f3d_test(NAME TestMorphColoring DATA SimpleMorph.gltf ARGS --scalar-coloring)
f3d_test(NAME TestAnimationUserMatrixColoring DATA InterpolationTest.glb ARGS --scalar-coloring --coloring-array=TEXCOORD_0 --animation-time=0.5)
f3d_test(NAME TestAnimationSkinColoring DATA SimpleSkin.gltf ARGS --scalar-coloring --coloring-array=WEIGHTS_0 --animation-time=2)
f3d_test(NAME TestAnimationMorphColoring DATA SimpleMorph.gltf ARGS --scalar-coloring --animation-time=0.5)
f3d_test(NAME TestAnimationInputChangeColoring DATA v_rock2.mdl ARGS --scalar-coloring --animation-time=0.01 --animation-index=1)
f3d_test(NAME TestAnimationUserMatrixPointSprites DATA BoxAnimated.gltf ARGS --point-sprites --point-sprites-size=50 --animation-time=2)
f3d_test(NAME TestAnimationInputChangePointSprites DATA v_rock2.mdl ARGS --point-sprites --point-sprites-size=50 --animation-time=0.01 --animation-index=1)

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
  f3d_test(NAME TestTextureColor DATA WaterBottle.glb ARGS --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo_mod.png --translucency-support)
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

# Needs splat sorting with compute shaders
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  if(NOT APPLE) # MacOS does not support compute shaders
    f3d_test(NAME Test3DGaussiansSplatting DATA small.splat ARGS -osy --up=-Y --point-sprites-size=1 --point-sprites-type=gaussian --camera-position=-3.6,0.5,-4.2)
    f3d_test(NAME TestDefaultConfigFileSPLAT DATA small.splat CONFIG config_build LONG_TIMEOUT UI)
    f3d_test(NAME TestThumbnailConfigFileSPLAT DATA small.splat CONFIG thumbnail_build LONG_TIMEOUT)

    f3d_test(NAME TestSPZDegree0 DATA hornedlizard_small_d0.spz ARGS -osy --point-sprites-size=1 --point-sprites-type=gaussian --camera-position=-3.6,0.5,4.2)
    f3d_test(NAME TestInvalidSH DATA invalidSH.vtp ARGS -osy --verbose REGEXP "Spherical harmonics array is not valid" NO_BASELINE)

    # Needs texture array support: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12112
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250513)
      f3d_test(NAME TestSPZDegree1 DATA hornedlizard_small_d1.spz ARGS -osy --point-sprites-size=1 --point-sprites-type=gaussian --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree2 DATA hornedlizard_small_d2.spz ARGS -osy --point-sprites-size=1 --point-sprites-type=gaussian --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree3 DATA hornedlizard_small_d3.spz ARGS -osy --point-sprites-size=1 --point-sprites-type=gaussian --camera-position=-3.6,0.5,4.2)

      f3d_test(NAME TestDefaultConfigFileSPZ DATA hornedlizard_small_d3.spz CONFIG config_build LONG_TIMEOUT UI)
      f3d_test(NAME TestThumbnailConfigFileSPZ DATA hornedlizard_small_d3.spz CONFIG thumbnail_build LONG_TIMEOUT)
    endif()
  endif()
endif()

# Test plugin fail code path
f3d_test(NAME TestPluginVerbose ARGS --verbose REGEXP "Loading plugin \"native\"" NO_BASELINE)
f3d_test(NAME TestPluginNonExistent ARGS --load-plugins=dummy REGEXP "Plugin failed to load" NO_BASELINE)
if(WIN32)
  set(_TEST_PLUGIN_INVALID_REGEXP "is not a valid Win32 application")
elseif(APPLE)
  set(_TEST_PLUGIN_INVALID_REGEXP "not a mach-o file")
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
  f3d_test(NAME TestConfigFileBuild DATA dragon.vtu CONFIG complex_build.json UI)
  f3d_test(NAME TestConfigStemBuild DATA dragon.vtu CONFIG complex_build UI)
  f3d_test(NAME TestConfigFileUpperCase DATA suzanne_upper.STL CONFIG complex_build)
  f3d_test(NAME TestConfigFileMultiFileSTL DATA mb/recursive/mb_1_0.vtp suzanne.stl ARGS --multi-file-mode=all CONFIG complex_build)
  f3d_test(NAME TestConfigFileMultiFileVTP DATA mb/recursive/mb_1_0.vtp suzanne.stl mb/recursive/mb_2_0.vtp ARGS --multi-file-mode=all CONFIG complex_build UI)

  file(COPY "${F3D_SOURCE_DIR}/resources/configs/config.d/" "${F3D_SOURCE_DIR}/plugins/native/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  f3d_test(NAME TestDefaultConfigFileVTU DATA dragon.vtu CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  f3d_test(NAME TestDefaultConfigFileVTI DATA vase_4comp.vti CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  f3d_test(NAME TestDefaultConfigFileSTL DATA suzanne.stl CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  f3d_test(NAME TestDefaultConfigFileTIFF DATA f3d.tif CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI) # Note: This tests config file order as camera_direction is set in different files
  f3d_test(NAME TestDefaultConfigFilePLY DATA suzanneRGBA.ply CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  f3d_test(NAME TestDefaultConfigFileQuakeMDL DATA zombie.mdl CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  f3d_test(NAME TestDefaultConfigFileAndCommand DATA suzanne.stl ARGS --up=-Y --camera-direction=-1,0.5,-1 CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  f3d_test(NAME TestDefaultConfigTranslucent DATA red_translucent_monkey.gltf CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

  file(COPY "${F3D_SOURCE_DIR}/resources/configs/thumbnail.d/" "${F3D_SOURCE_DIR}/plugins/native/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileVTU DATA dragon.vtu CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileVTI DATA vase_4comp.vti CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileSTL DATA suzanne.stl CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFilePLY DATA suzanneRGBA.ply CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileQuakeMDL DATA zombie.mdl CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
endif()

# color texture with opacity needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9467
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220811)
  f3d_test(NAME TestTextureColorWithOptions DATA WaterBottle.glb ARGS --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo_mod.png --color=1,1,0 --opacity=0.4 --translucency-support)
endif()

f3d_test(NAME TestGLTFImporterUnlit DATA UnlitTest.glb)
f3d_test(NAME TestMaterial DATA suzanne.ply ARGS --color=0.72,0.45,0.2 --metallic=0.7 --roughness=0.2)
f3d_test(NAME TestMaterialFullScene DATA WaterBottle.glb ARGS --color=0.9,0.1,0.1 --metallic=0.7 --roughness=0.2)
f3d_test(NAME TestMinBaseIOR DATA suzanne.ply ARGS --base-ior=1.0)
f3d_test(NAME TestMaxBaseIOR DATA suzanne.ply ARGS --base-ior=2.5)
f3d_test(NAME TestMetaData DATA pdiag.vtu ARGS -m UI)
f3d_test(NAME TestEdges DATA suzanne.ply ARGS -e)
f3d_test(NAME TestLineWidth DATA cow.vtk ARGS -e --line-width=5)
f3d_test(NAME TestLineWidthFullScene DATA suzanne.obj ARGS -e --line-width=3 --up=-Y)
f3d_test(NAME TestPointCloudFullScene DATA pointsCloud.gltf ARGS --point-size=20)
f3d_test(NAME TestTextureMatCapWithEdges DATA suzanne.ply ARGS -e --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png)

# Test enabling all animations
f3d_test(NAME TestAnimationAllAnimations DATA InterpolationTest.glb ARGS --animation-index=-1 --animation-time=1 --animation-progress)

# Test Verbose animation, no baseline needed
f3d_test(NAME TestVerboseAnimation DATA InterpolationTest.glb ARGS --verbose NO_BASELINE REGEXP "7: CubicSpline Translation")

# Test Animation index out of domain error
f3d_test(NAME TestVerboseAnimationIndexError DATA InterpolationTest.glb ARGS --animation-index=48 NO_BASELINE REGEXP "Specified animation index is greater than the highest possible animation index, enabling the first animation.")

# Test interactive animation and speed factor
f3d_test(NAME TestInteractionAnimation DATA f3d.glb ARGS --animation-progress INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationFast DATA f3d.glb ARGS --animation-progress --animation-speed-factor=1.5 INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationSlow DATA f3d.glb ARGS --animation-progress --animation-speed-factor=0.5 INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationFrameRate DATA f3d.glb ARGS --animation-progress --frame-rate=1 INTERACTION)#Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationCameraMovement DATA CameraAnimated.glb ARGS --camera-index=0 --animation-progress INTERACTION)#Space;MouseMovement;Space;
f3d_test(NAME TestInteractionAnimationInvert DATA f3d.glb ARGS --animation-speed-factor=-1 --animation-progress INTERACTION)#Space;Wait;Space;

# Test scrolling cheatsheet with no model by mouse wheel and by using scrollbar
f3d_test(NAME TestInteractionNoModelScrollWheel RESOLUTION 1000,600 NO_DATA_FORCE_RENDER INTERACTION UI)
f3d_test(NAME TestInteractionNoModelScrollBar RESOLUTION 1000,600 NO_DATA_FORCE_RENDER INTERACTION UI)

# A verbose test that needs animation index support
f3d_test(NAME TestVerboseAnimationWrongAnimationTimeHigh DATA BoxAnimated.gltf ARGS --animation-time=10 --verbose REGEXP "Animation time 10 is outside of range \\[0, 3\\.70833\\], using 3\\.70833" NO_BASELINE)
f3d_test(NAME TestVerboseAnimationWrongAnimationTimeLow DATA BoxAnimated.gltf ARGS --animation-time=-5 --verbose REGEXP "Animation time -5 is outside of range \\[0, 3\\.70833\\], using 0" NO_BASELINE)

# Verbose test for animation
f3d_test(NAME TestVerboseAnimationTimeRange DATA InterpolationTest.glb ARGS --verbose REGEXP "0, 1.66667" NO_BASELINE)
f3d_test(NAME TestCommandScriptVerboseMultiAnimationTimeRange SCRIPT TestCommandScriptVerboseMultiAnimationTimeRange.txt DATA InterpolationTest.glb ARGS --verbose REGEXP "0, 1.70833" NO_BASELINE)# cycle_animation x3
f3d_test(NAME TestVerboseQuakeMDLAnimationNoNamingScheme ARGS --verbose DATA v_rock2.mdl REGEXP "0: flame" NO_BASELINE)
f3d_test(NAME TestVerboseQuakeMDLGroupSkinUnsupported ARGS --verbose DATA groupskin.mdl REGEXP "Groupskin are not supported" NO_BASELINE)
f3d_test(NAME TestVerboseQuakeMDLInvalid ARGS --verbose DATA w_medkit_hl.mdl REGEXP "No frame read" NO_BASELINE)

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
f3d_test(NAME TestInteractionCheatsheetNoFile INTERACTION UI NO_DATA_FORCE_RENDER) #HXM
f3d_test(NAME TestInteractionCheatsheetScalars DATA dragon.vtu ARGS --scalar-coloring --coloring-component=-2 INTERACTION UI LONG_TIMEOUT) #HSSS
f3d_test(NAME TestInteractionCheatsheetOpacity DATA cow.vtp INTERACTION UI ARGS --opacity=0.5 RESOLUTION 300,700) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetAnimationName DATA InterpolationTest.glb ARGS --animation-index=6 INTERACTION UI) #HWWW
f3d_test(NAME TestInteractionCheatsheetConfigFile DATA dragon.vtu CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetSSAA DATA cow.vtp INTERACTION UI) #H;A;A

if(NOT F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestInteractionCheatsheetCentered DATA cow.vtp RESOLUTION 500,1500 INTERACTION UI LONG_TIMEOUT) #H
endif()

f3d_test(NAME TestCameraPersp DATA Cameras.gltf ARGS --camera-index=0)
f3d_test(NAME TestCameraOrtho DATA Cameras.gltf ARGS --camera-index=1)
f3d_test(NAME TestCameraIndexConfiguration DATA Cameras.gltf ARGS --camera-index=0  --camera-azimuth-angle=15 --camera-position=0.7,0.5,3)
f3d_test(NAME TestCameraIndexInvalid DATA Cameras.gltf ARGS --camera-index=3 REGEXP "is higher than the number of available camera" NO_BASELINE)
f3d_test(NAME TestCameraIndexNegative DATA Cameras.gltf ARGS --camera-index=-1 REGEXP "Invalid camera index" NO_BASELINE)

# Require improved importer support https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11303
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240910)
  f3d_test(NAME TestInvalidFileFileName DATA invalid.vtp ARGS --filename NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileInvalid DATA cow.vtp invalid.vtp dragon.vtu ARGS --multi-file-mode=all NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileInvalidFilesFileName DATA mb/mb_3_0.vtt invalid.vtp ARGS --multi-file-mode=all --filename NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileCameraIndex DATA Cameras.gltf CameraAnimated.glb ARGS --multi-file-mode=all --camera-index=2 --opacity=0.5 --translucency-support)
  f3d_test(NAME TestConsoleBadgeError DATA invalid.vtp NO_DATA_FORCE_RENDER UI)
endif()

# Test Verbose camera
f3d_test(NAME TestVerboseCamera DATA Cameras.gltf ARGS --camera-index=1 --verbose NO_RENDER REGEXP "0:.*1:")

f3d_test(NAME TestGLTFMorph DATA SimpleMorph.gltf)
f3d_test(NAME TestGLTFURI DATA Lantern/Lantern.gltf)

f3d_test(NAME TestNoBackground DATA cow.vtp ARGS --no-background)

# HDRI test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9767
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  f3d_test(NAME TestHDRI DATA suzanne.ply HDRI palermo_park_1k.hdr)
  f3d_test(NAME TestHDRICache DATA suzanne.ply HDRI palermo_park_1k.hdr DEPENDS TestHDRI)
  f3d_test(NAME TestHDRIBlur DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -u)
  f3d_test(NAME TestHDRIBlurCoCSmall DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=10 --camera-position=-20,0,20)
  f3d_test(NAME TestHDRIBlurCoCMedium DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=50 --camera-position=-20,0,20)
  f3d_test(NAME TestHDRIBlurCoCLarge DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=100 --camera-position=-20,0,20)
  f3d_test(NAME TestHDRIBlurCoCZero DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=0 --camera-position=-20,0,20 THRESHOLD 0.08) # Threshold is needed for legacy comparison method for VTK 9.3
  f3d_test(NAME TestHDRIBlurCoCNegative DATA suzanne.ply HDRI shanghai_bund_1k.hdr ARGS -u --blur-coc=-100 --camera-position=-20,0,20)
  f3d_test(NAME TestHDRIBlurRatio DATA suzanne.ply HDRI palermo_park_1k.hdr RESOLUTION 600,100 ARGS -u)
  f3d_test(NAME TestHDRIEdges DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -e)
  f3d_test(NAME TestHDRI8Bit DATA suzanne.ply HDRI f3d.tif ARGS --color=1.0,0.0,0.0)
  f3d_test(NAME TestHDRIOrient DATA suzanne.stl HDRI palermo_park_1k.hdr ARGS --up=+Z)
  f3d_test(NAME TestHDRIToneMapping DATA suzanne.ply HDRI palermo_park_1k.hdr TONE_MAPPING ARGS -t)
  f3d_test(NAME TestInteractionHDRIMove DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION THRESHOLD 0.05) #Shift+MouseRight; # Threshold is needed for legacy image comparison for VTK 9.3
  f3d_test(NAME TestInteractionHDRIBlur DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION) #U
  f3d_test(NAME TestInteractionHDRIReload DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION) #Up
  f3d_test(NAME TestInteractionHDRIChange DATA multi HDRI palermo_park_1k.hdr CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION) #Left

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
  f3d_test(NAME TestHDRIToneMappingSSAA DATA suzanne.ply HDRI palermo_park_1k.hdr TONE_MAPPING ARGS -t -a --anti-aliasing-mode=ssaa)
endif()

# Zoom factor was introduced in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9958
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20230221)
  f3d_test(NAME TestCameraZoomFactor DATA suzanne.obj ARGS --camera-direction=-1,-1,1 --camera-zoom-factor=1.5)
endif()

# VRML was fixed in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10235
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20230603)
  f3d_test(NAME TestVRMLImporter DATA bot2.wrl)
endif()

# TGA support for OBJ added in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11922
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250220)
  f3d_test(NAME TestOBJWithTGATexture DATA world_tga.obj)
endif()

if(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestRaytracingGLTF DATA WaterBottle.glb ARGS -rd --raytracing-samples=4)
  f3d_test(NAME TestRaytracingBackground DATA suzanne.ply ARGS -rd --raytracing-samples=4 --background-color=1,0,0 THRESHOLD 0.05) # Threshold needed because of difference in rendering in VTK 9.3
  f3d_test(NAME TestRaytracingPointCloud DATA pointsCloud.vtp ARGS -rd --raytracing-samples=4 --point-size=20 THRESHOLD 0.1) # Threshold needed because of difference in rendering in VTK 9.3
  f3d_test(NAME TestRaytracingDenoise DATA suzanne.ply ARGS -rd --raytracing-samples=4)
  f3d_test(NAME TestRaytracingNoDenoise DATA suzanne.ply ARGS -r --raytracing-samples=20)
  f3d_test(NAME TestVersionRaytracing ARGS --version REGEXP "Module Raytracing: ON")
  f3d_test(NAME TestInteractionRaytracingDenoise DATA suzanne.ply ARGS --raytracing-samples=4 INTERACTION) #RD
  f3d_test(NAME TestRaytracingScalarBar DATA dragon.vtu ARGS -rsbd --raytracing-samples=4 THRESHOLD 0.06) # Threshold needed because of difference in rendering in VTK 9.3 on macOS

  if(NOT F3D_MACOS_BUNDLE)
    f3d_test(NAME TestRaytracingDefaultConfigFile DATA dragon.vtu CONFIG config_build ARGS -rd --raytracing-samples=4 LONG_TIMEOUT TONE_MAPPING)
    f3d_test(NAME TestRaytracingThumbnailConfigFile DATA dragon.vtu CONFIG thumbnail_build ARGS -rd --raytracing-samples=4 LONG_TIMEOUT TONE_MAPPING)
  endif()

  f3d_test(NAME TestRaytracingNoBackground DATA suzanne.ply ARGS -rd --raytracing-samples=4 --no-background)

else(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestCommandScriptRaytracingNoRaytracing DATA suzanne.ply SCRIPT TestCommandScriptRaytracing.txt NO_BASELINE REGEXP "Raytracing options can't be used if F3D has not been built with raytracing")
endif()

if(F3D_MODULE_EXR)
  f3d_test(NAME TestVersionEXR ARGS --version REGEXP "Module OpenEXR: ON")
endif()

if(F3D_PLUGIN_BUILD_ALEMBIC)
  f3d_test(NAME TestABC DATA suzanne.abc ARGS -s --load-plugins=alembic)
  f3d_test(NAME TestABCNonFaceVarying DATA tetrahedron_non_facevarying_uv.abc ARGS -s --load-plugins=alembic)
  f3d_test(NAME TestABCAnimation DATA drop.abc ARGS --animation-time=2 --load-plugins=alembic --animation-progress)

  if(NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/alembic/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    f3d_test(NAME TestDefaultConfigFileAlembic DATA suzanne.abc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

    file(COPY "${F3D_SOURCE_DIR}/plugins/alembic/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_test(NAME TestThumbnailConfigFileAlembic DATA suzanne.abc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()

endif()

if(F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestOFF DATA teapot.off ARGS --up=+Z --load-plugins=assimp)
  f3d_test(NAME Test3MF DATA cube_gears.3mf ARGS --load-plugins=assimp)
  f3d_test(NAME TestFBX DATA phong_cube.fbx ARGS --load-plugins=assimp)
  f3d_test(NAME TestFBX16bits DATA 16bit.fbx ARGS --load-plugins=assimp)
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
    f3d_test(NAME TestAssimpValidation DATA Wolf.fbx ARGS --load-plugins=assimp REGEXP "Some of these files could not be loaded" NO_BASELINE)
  endif()
  f3d_test(NAME TestVerboseCameraAssimp DATA duck.dae ARGS --verbose --load-plugins=assimp NO_BASELINE REGEXP "camera1")

  if(NOT APPLE OR VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
    f3d_test(NAME TestDXF DATA PinkEggFromLW.dxf ARGS --background-color=1,1,1 -p --load-plugins=assimp)
  endif()

  f3d_test(NAME TestVerboseAssimp DATA duck.fbx ARGS --verbose --load-plugins=assimp NO_BASELINE REGEXP "LOD3sp")
  f3d_test(NAME TestVerboseAssimpAnimationIndexError DATA animatedLights.fbx ARGS --load-plugins=assimp --animation-index=48 NO_BASELINE REGEXP "Specified animation index is greater than the highest possible animation index, enabling the first animation.")
  f3d_test(NAME TestAssimpAnimationNegativeIndex DATA animatedLights.fbx ARGS --load-plugins=assimp --animation-index=-113 --animation-time=2 --animation-progress)
  f3d_test(NAME TestTGATextureFBX DATA duck.fbx ARGS --load-plugins=assimp)
  f3d_test(NAME TestDAE DATA duck.dae ARGS --load-plugins=assimp)
  f3d_test(NAME TestX DATA anim_test.x ARGS --load-plugins=assimp)

  # This test baseline is incorrect because of https://github.com/f3d-app/f3d/issues/603
  # It will need to be changed when fixed
  f3d_test(NAME TestFBXNormalMapping DATA normalMapping.fbx ARGS --load-plugins=assimp)

  # Tests using embedded textures
  f3d_test(NAME TestEmbeddedTextureFBX DATA texturedCube.fbx ARGS --load-plugins=assimp)
  f3d_test(NAME TestFBXAnimation DATA animatedWorld.fbx ARGS --load-plugins=assimp --animation-time=2 --animation-progress)
  f3d_test(NAME TestFBXAnimationLights DATA animatedLights.fbx ARGS --load-plugins=assimp --animation-time=1.8 --animation-progress)
  f3d_test(NAME TestFBXAnimationCamera DATA animatedCamera.fbx ARGS --load-plugins=assimp --camera-index=0 --animation-index=0 --animation-time=3 --animation-progress)
  f3d_test(NAME TestDAEAnimationLights DATA animatedLights.dae ARGS --load-plugins=assimp --animation-time=1.8 --animation-progress)

  if("${F3D_ASSIMP_VERSION}" VERSION_GREATER_EQUAL "5.4.3")
    f3d_test(NAME TestFBXBone DATA animation_with_skeleton.fbx ARGS --load-plugins=assimp --camera-position=1.90735e-06,0,11007.8 --camera-focal-point=1.90735e-06,0,-8.9407e-08)
    f3d_test(NAME TestFBXBoneAnimation DATA animation_with_skeleton.fbx ARGS --load-plugins=assimp --camera-position=1.90735e-06,0,11007.8 --camera-focal-point=1.90735e-06,0,-8.9407e-08 --animation-time=0.5 --animation-progress)
    f3d_test(NAME TestInteractionAnimationFBXBone DATA animation_with_skeleton.fbx ARGS --load-plugins=assimp --camera-position=0,0,14000 --camera-focal-point=0,0,0 INTERACTION)#Space;Wait;Space;
  endif()

  # The visible boxes are not located at the same position in Assimp 6.0 for some reason
  if("${F3D_ASSIMP_VERSION}" VERSION_GREATER_EQUAL "6.0.1")
    f3d_test(NAME TestFBXSkinningAnimation DATA punch.fbx ARGS --load-plugins=assimp --animation-time=1 --animation-progress)
  endif()

  if(NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/assimp/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    f3d_test(NAME TestDefaultConfigFileAssimpFBX DATA phong_cube.fbx CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAssimpDXF DATA PinkEggFromLW.dxf ARGS -p CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAssimpOFF DATA teapot.off CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAssimpDAE DATA duck.dae CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

    file(COPY "${F3D_SOURCE_DIR}/plugins/assimp/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_test(NAME TestThumbnailConfigFileAssimpFBX DATA phong_cube.fbx CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
    f3d_test(NAME TestThumbnailConfigFileAssimpDXF DATA PinkEggFromLW.dxf ARGS -p CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
    f3d_test(NAME TestThumbnailConfigFileAssimpOFF DATA teapot.off CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
    f3d_test(NAME TestThumbnailConfigFileAssimpDAE DATA duck.dae CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()

  f3d_test(NAME TestAssimpVerbose DATA duck.fbx ARGS --verbose --load-plugins=assimp REGEXP "Number of points: 12636" NO_BASELINE)

  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
    f3d_test(NAME TestAssimpMetaDataImporter DATA duck.fbx ARGS --load-plugins=assimp -m UI)
  endif()

endif()

if(F3D_PLUGIN_BUILD_DRACO)
  f3d_test(NAME TestDRACO DATA suzanne.drc ARGS --load-plugins=draco)
  f3d_test(NAME TestDRACOColoring DATA suzanne.drc ARGS --scalar-coloring --coloring-component=0 --load-plugins=draco)

  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10884
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240214)
    f3d_test(NAME TestGLTFDracoImporter DATA Box_draco.gltf ARGS --load-plugins=draco --camera-position=-1.6,1.3,2.7)
    f3d_test(NAME TestGLTFDracoImporterWithoutCompression DATA BoxAnimated.gltf ARGS --load-plugins=draco --animation-time=2 --animation-progress)
  endif()

  if(NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/draco/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    f3d_test(NAME TestDefaultConfigFileDraco DATA suzanne.drc CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

    file(COPY "${F3D_SOURCE_DIR}/plugins/draco/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_test(NAME TestThumbnailConfigFileDraco DATA suzanne.drc CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()

endif()

if(F3D_PLUGIN_BUILD_HDF)
  if (VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
    f3d_test(NAME TestVTKHDF DATA blob.vtkhdf ARGS --load-plugins=hdf -s)
    f3d_test(NAME TestAMRDataSet DATA amr.vtkhdf ARGS --load-plugins=hdf -s)
  endif()

  f3d_test(NAME TestExodus DATA disk_out_ref.ex2 ARGS --load-plugins=hdf -s --camera-position=-11,-2,-49)

  f3d_test(NAME TestExodusConfig DATA disk_out_ref.ex2 CONFIG ${F3D_SOURCE_DIR}/testing/configs/exodus.json ARGS -s --camera-position=-11,-2,-49)

  # Test Generic Importer Verbose animation. Regex contains the time range.
  f3d_test(NAME TestVerboseGenericImporterAnimation DATA small.ex2 ARGS --load-plugins=hdf --verbose NO_BASELINE REGEXP "0, 0.00429999")

  # Test animation with generic importer and coloring
  f3d_test(NAME TestAnimationGenericImporter DATA small.ex2 ARGS -sb --load-plugins=hdf --animation-time=0.003 --animation-progress)

  # Test animation with generic importer, coloring and point sprites
  f3d_test(NAME TestAnimationGenericImporterPointSprites DATA small.ex2 ARGS -sbo --load-plugins=hdf --animation-time=0.003 --animation-progress)

  # Test animation with generic importer, coloring and a custom scalar range
  f3d_test(NAME TestAnimationGenericImporterScalarRange DATA small.ex2 ARGS -sb --load-plugins=hdf --animation-time=0.003 --animation-progress --coloring-range=0,1e7)

  # Test Generic Importer Verbose animation with a single frame.
  f3d_test(NAME TestVerboseAnimationSingleTimestep DATA single_timestep.e ARGS --load-plugins=hdf --verbose NO_BASELINE REGEXP "0, 0")

  # Test no render animation time. Regex contains a part of the range of the VEL_ field.
  f3d_test(NAME TestNoRenderAnimation DATA small.ex2 ARGS  --load-plugins=hdf --animation-time=0.003 REGEXP "-994.473, 33.9259" NO_RENDER)

  # Test animation time clamping
  f3d_test(NAME TestAnimationTimeLimitsHigh DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=10)
  f3d_test(NAME TestAnimationTimeLimitsLow DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=-5)
  f3d_test(NAME TestAnimationTimeLimitsHighNoWarning DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=0.0043001 REGEXP_FAIL "outside of range" NO_RENDER)
  f3d_test(NAME TestAnimationTimeLimitsLowNoWarning DATA small.ex2 ARGS ARGS --load-plugins=hdf --animation-time=-0.000001 REGEXP_FAIL "outside of range" NO_RENDER)

  # Test if negative range is respected when loading a file without specifying the animation time
  f3d_test(NAME TestTimeRangeLessThanZeroNoAnimationTime DATA negative_range_animated.e ARGS -s --load-plugins=hdf)

  # Test if the animation-time works when set to zero and time range[0] is less than zero
  f3d_test(NAME TestTimeRangeLessThanZeroWithAnimationTime DATA negative_range_animated.e ARGS -s --load-plugins=hdf --animation-time=0)

  # Test if a negative animation-time works when time range[0] is less than zero
  f3d_test(NAME TestTimeRangeLessThanZeroNegativeAnimationTime DATA negative_range_animated.e ARGS -s --load-plugins=hdf --animation-time=-3)

  if(NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/hdf/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    f3d_test(NAME TestDefaultConfigFileExodus DATA disk_out_ref.ex2 CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

    file(COPY "${F3D_SOURCE_DIR}/plugins/hdf/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_test(NAME TestThumbnailConfigFileExodus DATA disk_out_ref.ex2 CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)

    if (VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
      f3d_test(NAME TestDefaultConfigFileVTKHDF DATA blob.vtkhdf CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
      f3d_test(NAME TestThumbnailConfigFileVTKHDF DATA blob.vtkhdf CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
    endif()
  endif()

  if (NOT F3D_PLUGINS_STATIC_BUILD AND BUILD_SHARED_LIBS)

    # Test --load-plugins with the name of a dynamic plugin
    f3d_test(NAME TestPluginName DATA disk_out_ref.ex2 ARGS --load-plugins=hdf --verbose NO_BASELINE REGEXP "Loaded plugin hdf from")

    # Test --load-plugins with a full path plugin
    f3d_test(NAME TestPluginFullPath DATA disk_out_ref.ex2 ARGS --verbose --load-plugins "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_MODULE_PREFIX}f3d-plugin-hdf${CMAKE_SHARED_MODULE_SUFFIX}" NO_BASELINE REGEXP "Loaded plugin hdf from")
  endif()

endif()

if(F3D_PLUGIN_BUILD_OCCT)
  f3d_test(NAME TestSTEP DATA f3d.stp ARGS --load-plugins=occt --up=+Z)
  f3d_test(NAME TestIGES DATA f3d.igs ARGS --load-plugins=occt --up=+Z)
  f3d_test(NAME TestBREP DATA f3d.brep ARGS --load-plugins=occt --up=+Z)
  f3d_test(NAME TestBinaryBREP DATA f3d.bin.brep ARGS --load-plugins=occt --up=+Z)
  f3d_test(NAME TestSTEPDefines DATA cheese.stp ARGS --load-plugins=occt -DSTEP.linear_deflection=0.5 -DSTEP.angular_deflection=0.9 -DSTEP.relative_deflection=1)
  f3d_test(NAME TestIGESDefines DATA cheese.igs ARGS --load-plugins=occt -DIGES.read_wire=0 --line-width=5 --up=+Z)
  f3d_test(NAME TestBREPDefines DATA cheese.brep ARGS --load-plugins=occt -DBREP.linear_deflection=100 --up=+Z)

  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
    f3d_test(NAME TestInvalidBREP DATA invalid.brep ARGS --verbose --load-plugins=occt REGEXP "failed to load scene" NO_BASELINE)
  endif()

  if(F3D_PLUGIN_OCCT_COLORING_SUPPORT)
    f3d_test(NAME TestXBF DATA f3d.xbf ARGS --load-plugins=occt --up=+Z)
    f3d_test(NAME TestXBFDefines DATA cheese.xbf ARGS --load-plugins=occt -DXBF.angular_deflection=1 -DXBF.relative_deflection=1 --up=+Z)
    f3d_test(NAME TestXCAFColors DATA xcaf-colors.stp ARGS --load-plugins=occt -csy --up=+Z --line-width=3 --camera-direction=-1,-1,-1)
    f3d_test(NAME TestXCAFColorsXBF DATA xcaf-colors.xbf ARGS --load-plugins=occt -csy --up=+Z --line-width=3 --camera-direction=-1,-1,-1)
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
      f3d_test(NAME TestInvalidXBF DATA invalid.xbf ARGS --verbose --load-plugins=occt REGEXP "failed to load scene" NO_BASELINE)
    endif()

    if (NOT F3D_MACOS_BUNDLE)
      file(COPY "${F3D_SOURCE_DIR}/plugins/occt/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
      f3d_test(NAME TestDefaultConfigFileOCCT DATA f3d.stp CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

      file(COPY "${F3D_SOURCE_DIR}/plugins/occt/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
      f3d_test(NAME TestThumbnailConfigFileOCCT DATA f3d.stp CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
    endif()
  endif()
endif()

if(F3D_PLUGIN_BUILD_USD)
  f3d_test(NAME TestUSD DATA suzanne.usd ARGS --load-plugins=usd)
  f3d_test(NAME TestUSDAPrimitives DATA primitives.usda ARGS --load-plugins=usd)
  f3d_test(NAME TestUSDAPrimitivesZAxis DATA primitivesZ.usda ARGS --load-plugins=usd)
  f3d_test(NAME TestUSDAInstancing DATA instancing.usda ARGS --load-plugins=usd)
  f3d_test(NAME TestUSDAGlyphs DATA glyphs.usda ARGS --load-plugins=usd)
  f3d_test(NAME TestUSDInvalid DATA invalid.usd REGEXP "Stage failed to open" ARGS --verbose --load-plugins=usd NO_BASELINE)
  f3d_test(NAME TestUSDPurpose DATA purpose.usdc ARGS --load-plugins=usd)
  f3d_test(NAME TestUSDInterpolation DATA two_quads_interp.usda ARGS --load-plugins=usd)

  # This test is there to test occlusion texture and face-varying point data
  # TODO: Note that the result looks incorrect because of face-varying attributes and must be fixed later
  f3d_test(NAME TestUSDTeapot DATA Teapot.usd ARGS --load-plugins=usd)

  f3d_test(NAME TestUSDZAnimated DATA AnimatedCube.usdz ARGS --load-plugins=usd --animation-time=0.3 --animation-progress)
  f3d_test(NAME TestUSDZRigged DATA RiggedSimple.usdz ARGS --load-plugins=usd --animation-time=0.3)
  f3d_test(NAME TestUSDZMaterials DATA McUsd.usdz ARGS --load-plugins=usd --camera-position=1055,912,-247 --camera-focal-point=69,173,63 THRESHOLD 0.3) # High threshold because of legacy comparison methods in VTK 9.3
  f3d_test(NAME TestUSDZMaterialsInterationReload DATA McUsd.usdz ARGS --load-plugins=usd INTERACTION NO_BASELINE) # Up
  f3d_test(NAME TestUSDZMemEXR DATA small.usdz ARGS --load-plugins=usd)

  if(NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/usd/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    f3d_test(NAME TestDefaultConfigFileUSD DATA suzanne.usd CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

    file(COPY "${F3D_SOURCE_DIR}/plugins/usd/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_test(NAME TestThumbnailConfigFileUSD DATA suzanne.usd CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()
endif()

if(F3D_PLUGIN_BUILD_VDB)
  f3d_test(NAME TestVDBVolume DATA icosahedron.vdb ARGS --load-plugins=vdb --volume --volume-inverse)
  f3d_test(NAME TestVDBPoints DATA sphere_points.vdb ARGS --load-plugins=vdb -o)
  f3d_test(NAME TestVDBVerbose DATA icosahedron.vdb ARGS --load-plugins=vdb REGEXP "PartitionedDataSet" NO_RENDER)
  f3d_test(NAME TestVDBDefinesDownsamplingFactor DATA icosahedron.vdb ARGS --load-plugins=vdb -DVDB.downsampling_factor=0.2 --volume --volume-inverse)
  f3d_test(NAME TestVDBDefinesInexistent DATA icosahedron.vdb ARGS --load-plugins=vdb -Dvdb.downsampling_factor=0.2 REGEXP "did you mean 'VDB.downsampling_factor'" NO_BASELINE)
  f3d_test(NAME TestVDBDefinesDownsamplingFactorParseError DATA icosahedron.vdb ARGS --load-plugins=vdb -DVDB.downsampling_factor=abcde --verbose REGEXP "Could not parse VDB.downsampling_factor" NO_BASELINE)
  f3d_test(NAME TestVDBDefinesDownsamplingFactorOutOfRangeError DATA icosahedron.vdb ARGS --load-plugins=vdb -DVDB.downsampling_factor=${_outOfRangeDoubleStr} --verbose REGEXP "VDB.downsampling_factor out of range" NO_BASELINE)
  f3d_test(NAME TestVDBCommandScriptReaderOptions DATA icosahedron.vdb ARGS --load-plugins=vdb --volume --volume-inverse SCRIPT TestVDBCommandScriptReaderOptions.txt) # set_reader_option VDB.downsampling_factor 0.2; reload_current_file_group

  if(NOT F3D_MACOS_BUNDLE)
    file(COPY "${F3D_SOURCE_DIR}/plugins/vdb/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
    f3d_test(NAME TestDefaultConfigFileVDB DATA icosahedron.vdb CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)

    file(COPY "${F3D_SOURCE_DIR}/plugins/vdb/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
    f3d_test(NAME TestThumbnailConfigFileVDB DATA icosahedron.vdb CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  endif()
endif ()

if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestMultiplePluginsLoad DATA cow.vtp ARGS --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load")
endif()

f3d_test(NAME TestFinalShaderNegative DATA cow.vtp ARGS --final-shader "vec4 pixel(vec2 uv){return vec4(vec3(1.0) - texture(source, uv).rgb, 1.0)\\\\\\\;}")
f3d_test(NAME TestFinalShaderNegativeFileName DATA cow.vtp ARGS --filename --final-shader "vec4 pixel(vec2 uv){return vec4(vec3(1.0) - texture(source, uv).rgb, 1.0)\\\\\\\;}" UI)
f3d_test(NAME TestFinalShaderUndefined DATA cow.vtp ARGS --final-shader "undefined" REGEXP "Final shader must define a function" NO_BASELINE)
f3d_test(NAME TestFinalShaderCompilationFailure DATA cow.vtp ARGS --final-shader "vec4 pixel(vec2 uv){}" --verbose REGEXP " build the shader program" NO_BASELINE)

## Interaction Tests
# Test hotkeys
f3d_test(NAME TestInteractionPostFX DATA cow.vtp INTERACTION TONE_MAPPING LONG_TIMEOUT) #PQAAAAT
f3d_test(NAME TestInteractionActors DATA cow.vtp INTERACTION UI) #EXGMN
f3d_test(NAME TestInteractionTimer DATA cow.vtp NO_BASELINE INTERACTION UI LONG_TIMEOUT) #Z
f3d_test(NAME TestInteractionTrackball DATA cow.vtp ARGS --interaction-trackball INTERACTION) #Mouse movements;
f3d_test(NAME TestInteractionTrackballBind DATA cow.vtp INTERACTION) #K#Mouse movements;
f3d_test(NAME TestInteractionCycleCell DATA waveletArrays.vti INTERACTION LONG_TIMEOUT) #VCCC
f3d_test(NAME TestInteractionCycleComp DATA dragon.vtu INTERACTION) #SYYYY
f3d_test(NAME TestInteractionCycleScalars DATA dragon.vtu INTERACTION) #BSSSS
f3d_test(NAME TestInteractionCycleCellInvalidIndex DATA waveletArrays.vti INTERACTION) #SSC
f3d_test(NAME TestInteractionVolumeCycle DATA waveletArrays.vti ARGS INTERACTION) #VSS
f3d_test(NAME TestInteractionVolumeAfterColoring DATA waveletArrays.vti ARGS INTERACTION) #SSSV
f3d_test(NAME TestInteractionVolumeInverse DATA HeadMRVolume.mhd ARGS --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 INTERACTION THRESHOLD 0.05)#VI #Small rendering differences due to volume rendering
f3d_test(NAME TestInteractionMultiFileVolume DATA multi ARGS --multi-file-mode=all INTERACTION) #SSVB
f3d_test(NAME TestInteractionPointCloud DATA pointsCloud.vtp ARGS --point-sprites-size=20 INTERACTION) #O
f3d_test(NAME TestInteractionDirectory DATA mb INTERACTION ARGS --scalar-coloring) #Right;Right;Right;Left;Up;
f3d_test(NAME TestInteractionDirectoryLoop DATA mb/recursive INTERACTION ARGS --scalar-coloring --filename UI) #Left;Left;Left;Left;Left;
f3d_test(NAME TestInteractionDirectoryEmpty DATA mb INTERACTION NO_DATA_FORCE_RENDER UI) #Right;Right;Right;
f3d_test(NAME TestInteractionDirectoryEmptyVerbose DATA mb ARGS --verbose NO_BASELINE INTERACTION REGEXP "is not a file of a supported file format") #Right;Right;Right;HMCSY
f3d_test(NAME TestInteractionAnimationNotStopped DATA InterpolationTest.glb NO_BASELINE INTERACTION)#Space;
f3d_test(NAME TestInteractionResetCamera DATA dragon.vtu INTERACTION LONG_TIMEOUT)#MouseMovements;Return;
f3d_test(NAME TestInteractionResetCameraWithCameraIndex DATA CameraAnimated.glb ARGS --camera-index=0 INTERACTION)#MouseMovements;Return;
f3d_test(NAME TestInteractionTensorsCycleComp DATA tensors.vti ARGS --scalar-coloring --coloring-component=-2 INTERACTION) #SYYYYYYYYYY
f3d_test(NAME TestInteractionCycleScalarsCompCheck DATA dragon.vtu ARGS -b --scalar-coloring --coloring-component=2 INTERACTION) #S
f3d_test(NAME TestInteractionConfigFileNoColorBar DATA multi CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION) #Right;Right;Left
f3d_test(NAME TestInteractionConfigFileBindings DATA dragon.vtu CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #Ctrl+Shift+O;Ctrl+O;Shift+O;O;3
f3d_test(NAME TestInteractionConfigFileMulti DATA multi CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #SY;Right;XG;Right;N;Right;Right
f3d_test(NAME TestInteractionConfigFileAndCommand DATA multi ARGS -o CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #OX;Right;N;Right;Right;Right
f3d_test(NAME TestInteractionConfigFileImperative DATA dragon.vtu suzanne.stl ARGS --edges CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json INTERACTION) #E;Right
f3d_test(NAME TestInteractionConfigFileImperativeNoData CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json INTERACTION NO_DATA_FORCE_RENDER UI) #X;Up
f3d_test(NAME TestInteractionCycleVerbose DATA dragon.vtu ARGS --verbose -s NO_BASELINE INTERACTION REGEXP "Not coloring")#SSSSYC
f3d_test(NAME TestInteractionEmptyDrop INTERACTION REGEXP "Drop event without any provided files.")#DropEvent Empty;
f3d_test(NAME TestInteractionCameraUpdate DATA dragon.vtu INTERACTION) #MouseWheel;MouseWheel;MouseWheel;S
f3d_test(NAME TestInteractionFocalPointPickingDefault DATA dragon.vtu INTERACTION LONG_TIMEOUT)
f3d_test(NAME TestInteractionFocalPointPickingShift DATA dragon.vtu INTERACTION LONG_TIMEOUT)
f3d_test(NAME TestInteractionFocalPointPickingPoints DATA pointsCloud.vtp INTERACTION THRESHOLD 0.05) # Threshold needed because sometime a point does not appear
f3d_test(NAME TestInteractionLightIntensity DATA dragon.vtu INTERACTION LONG_TIMEOUT)
f3d_test(NAME TestInteractionMultiFileColoring DATA mb/recursive ARGS --multi-file-mode=all INTERACTION) #SSSB
f3d_test(NAME TestInteractionOpacity DATA dragon.vtu INTERACTION)
f3d_test(NAME TestInteractionInitUpVectorOption DATA dragon.vtu ARGS --up=-Y INTERACTION) #Small drag left, camera should maintain orientation
f3d_test(NAME TestInteractionDragRotateVertical DATA offset-flat-box.glb ARGS -g -x --up=y INTERACTION) # Drag down, should look straight down and avoid gimbal lock
f3d_test(NAME TestInteractionReload DATA dragon.vtu ARGS -e INTERACTION) #Up;
f3d_test(NAME TestInteractionLoadParentDirectory DATA multi/dragon.vtu ARGS --filename INTERACTION UI) #Down;
f3d_test(NAME TestInteractionEmptyLoadParentDirectory INTERACTION NO_BASELINE REGEXP "No files loaded, no rendering performed") #Down;
f3d_test(NAME TestInteractionMultiFileLoadParentDirectory DATA mb/mb_0_0.vtu ARGS --multi-file-mode=all --filename INTERACTION UI) #Down;
f3d_test(NAME TestInteractionInvertZoom DATA suzanne.ply ARGS --invert-zoom INTERACTION)
f3d_test(NAME TestInteractionCameraHotkeys DATA cow.vtp INTERACTION)
f3d_test(NAME TestInteractionZoomToMouse DATA cow.vtp INTERACTION)
f3d_test(NAME TestInteractionOrthographicProjection DATA cow.vtp INTERACTION) #5;5
f3d_test(NAME TestInteractionZoomToggleOrthographicProjection DATA cow.vtp INTERACTION) #MouseWheel;5;Mousewheelx6;5
f3d_test(NAME TestInteractionRotateCameraMinus90 DATA f3d.glb INTERACTION)
f3d_test(NAME TestInteractionRotateCamera90 DATA f3d.glb INTERACTION)
f3d_test(NAME TestInteractionRollCameraRotation DATA f3d.glb ARGS -g INTERACTION) #1;4;LeftMouse;MouseMovements
f3d_test(NAME TestInteractionPanWithShift DATA f3d.glb INTERACTION) #Shift;LeftMouse;MouseMovements
f3d_test(NAME TestInteractionConsoleOpen DATA f3d.glb INTERACTION UI) #Escape
f3d_test(NAME TestInteractionConsoleOpenExit DATA f3d.glb REGEXP "Interactor has been stopped" INTERACTION NO_BASELINE UI) #Escape;exit;Return
f3d_test(NAME TestInteractionConsoleOpenWarningKeyboard DATA f3d.glb ARGS --position=0 INTERACTION UI) #Escape
f3d_test(NAME TestInteractionConsoleOpenWarningClick DATA f3d.glb ARGS --position=0 INTERACTION UI) #Click badge
f3d_test(NAME TestInteractionConsoleToggleGrid DATA f3d.glb INTERACTION UI) #Escape;toggle render.grid.enable;Return;Escape
f3d_test(NAME TestInteractionConsoleInvalidCommand DATA f3d.glb INTERACTION UI) #Escape;foo;Return
f3d_test(NAME TestInteractionConsoleInvalidOption DATA f3d.glb INTERACTION UI) #Escape;toggle foo;Return
f3d_test(NAME TestInteractionConsoleClear DATA dragon.vtu INTERACTION UI) #Escape;e;Escape;printt;BackSpace;_scee;Left;n;Right;_info;Return;clear;Return
f3d_test(NAME TestInteractionConsoleTypingSceneInfo DATA f3d.glb INTERACTION UI) #Escape;e;Escape;printt;BackSpace;_scee;Left;Right;_info;Return
f3d_test(NAME TestInteractionConsoleReload DATA f3d.glb INTERACTION UI) #Escape;reload_current_file_group;Return
f3d_test(NAME TestInteractionConsoleLoadNext DATA f3d.glb cow.vtp INTERACTION UI) #Escape;load_next_file_group;Return
f3d_test(NAME TestInteractionConsoleRender ARGS --coloring-by-cells DATA waveletMaterial.vti INTERACTION UI) #Escape;toggle_volume_rendering;Return
f3d_test(NAME TestInteractionConsoleCamera DATA f3d.glb INTERACTION UI) #Escape;set_camera top;Return
f3d_test(NAME TestInteractionConsoleScrollbar DATA f3d.glb INTERACTION UI) #Escape;a;Enter;Grab scrollbar
f3d_test(NAME TestInteractionConsoleEmptyCommand DATA f3d.glb INTERACTION UI) #Escape;Enter
f3d_test(NAME TestInteractionConsoleAutoComplete DATA BoxAnimated.gltf INTERACTION UI) #Escape;Tab;to;Tab;Tab;_a;Tab;Enter
f3d_test(NAME TestInteractionConsoleAutoCompleteNoMatch DATA BoxAnimated.gltf INTERACTION UI) #Escape;zz;Tab
f3d_test(NAME TestInteractionConsoleAutoCompleteCandidates DATA BoxAnimated.gltf INTERACTION UI) #Escape;Tab;to;Tab
f3d_test(NAME TestInteractionConsoleAutoCompleteHide DATA BoxAnimated.gltf INTERACTION UI) #Escape;Tab;to;Tab;Esc;Esc;Esc
f3d_test(NAME TestInteractionConsoleHistory DATA BoxAnimated.gltf INTERACTION UI) #Escape;ro;Tab;45;Enter;set_c;Tab;top;Enter;Up;Up;Enter;Up;Up;Up;Up;Down;Down;Down;Down
f3d_test(NAME TestInteractionConsoleHistoryEmpty DATA BoxAnimated.gltf INTERACTION UI)#Escape;Up;Down
f3d_test(NAME TestInteractionConsoleHistoryHide DATA BoxAnimated.gltf INTERACTION UI)#Escape;a;Enter;b;Enter;Up;Escape;Escape;Escape;Up
f3d_test(NAME TestInteractionMinimalConsoleOpen DATA f3d.glb INTERACTION UI) #Colon
f3d_test(NAME TestInteractionMinimalConsoleEscape DATA f3d.glb INTERACTION UI) #Colon;Escape
f3d_test(NAME TestInteractionMinimalConsoleReturn DATA f3d.glb INTERACTION UI) #Colon;Return
f3d_test(NAME TestInteractionMinimalConsoleValidCommand DATA f3d.glb INTERACTION UI) #Colon;set_camera top;Return
f3d_test(NAME TestInteractionMinimalConsoleInvalidCommand DATA f3d.glb INTERACTION UI) #Colon;foo;Return
f3d_test(NAME TestInteractionMinimalConsoleResize DATA f3d.glb INTERACTION UI) #Colon;foo;Return;Colon
f3d_test(NAME TestInteractionMinimalConsoleBadgeRemains DATA f3d.glb INTERACTION UI) #Colon;foo;Return;Colon;Escape

if (F3D_MODULE_TINYFILEDIALOGS)
f3d_test(NAME TestInteractionFileOpen INTERACTION NO_BASELINE ENV CTEST_OPEN_DIALOG_FILE=${F3D_SOURCE_DIR}/testing/data/cow.vtp)
endif ()
# Test camera preserving/resetting when switching files in interaction mode
f3d_test(NAME TestInteractionSwitchFileNoCameraKeeping DATA cow.vtp cowlow.vtp INTERACTION)
f3d_test(NAME TestInteractionSwitchFilePrevCameraKeeping DATA cow.vtp cowlow.vtp INTERACTION)
f3d_test(NAME TestInteractionSwitchFileNextCameraKeeping DATA cow.vtp cowlow.vtp INTERACTION)

# Progress test
f3d_test(NAME TestProgress DATA cow.vtp ARGS --loading-progress NO_BASELINE)
f3d_test(NAME TestProgressScene DATA WaterBottle.glb ARGS --loading-progress NO_BASELINE)
f3d_test(NAME TestInteractionProgressReload DATA cow.vtp ARGS --loading-progress NO_BASELINE INTERACTION) #Up;Up;Up;Up

f3d_test(NAME TestInteractionAnimationCycleAnimation DATA InterpolationTest.glb INTERACTION) #WWWWWWWWWWW;Space;Space;
f3d_test(NAME TestInteractionCycleAnimationNoAnimation DATA cow.vtp INTERACTION NO_BASELINE) #W

f3d_test(NAME TestInteractionDropFiles ARGS -n INTERACTION_CONFIGURE UI)#X;DropEvent cow.vtp;DropEvent dragon.vtu suzanne.stl;
f3d_test(NAME TestInteractionMultiFileDrop ARGS --multi-file-mode=all -e INTERACTION_CONFIGURE) #DropEvent mb_1_0.vtp mb_2_0.vtp
f3d_test(NAME TestInteractionDropSameFiles ARGS -x INTERACTION_CONFIGURE) #DropEvent cow.vtp;#DropEvent dragon.vtu;#DropEvent cow.vtp#DropEvent cow.vtp;

# A proper test for this is not possible because of the double quotes
f3d_test(NAME TestInteractionDropFileWithQuotes ARGS -n INTERACTION REGEXP "\"'`Quotes\"'`.stl does not exist" NO_BASELINE)#X;DropEvent "'`Quotes"'`.stl";

if(WIN32)
  # Windows specific drop test, using backslashes
  f3d_test(NAME TestInteractionDropFileBackslashes ARGS -n INTERACTION_CONFIGURE)#X;DropEvent path\to\cow.vtp;
endif()

# HDRI test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9767
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
  f3d_test(NAME TestInteractionDropHDRI INTERACTION_CONFIGURE LONG_TIMEOUT)#X;DropEvent dragon.vtu;DropEvent palermo.hdr;
  f3d_test(NAME TestInteractionDropHDRIInvert INTERACTION_CONFIGURE LONG_TIMEOUT)#X;DropEvent palermo.hdr;DropEvent dragon.vtu;
  f3d_test(NAME TestInteractionDropHDRIMulti INTERACTION_CONFIGURE LONG_TIMEOUT)#X;DropEvent dragon.vtu palermo.hdr;

  # Test modified drops, this test rendering is impacted by https://github.com/f3d-app/f3d/issues/1558
  # Empty drop is for coverage
  f3d_test(NAME TestInteractionDropHDRIModifiers INTERACTION_CONFIGURE LONG_TIMEOUT)#CTRL+DropEvent f3d.tif;SHIFT+DropEvent;SHIFT+DropEvent palermo.tif;SYYYY

  # Test interactive animation and dropping HDRI
  f3d_test(NAME TestInteractionAnimationDropHDRI DATA InterpolationTest.glb ARGS --animation-index=-1 --animation-progress INTERACTION_CONFIGURE LONG_TIMEOUT)#Space;DropEvent palermo.hdr;Space;

  if(F3D_MODULE_EXR)
    f3d_test(NAME TestInteractionDropHDRIExr INTERACTION_CONFIGURE LONG_TIMEOUT)#X;DropEvent kloofendal.exr;DropEvent dragon.vtu;
  endif()
endif()

## Tests to increase coverage
# Output option test
f3d_test(NAME TestOutput DATA cow.vtp NO_BASELINE)
f3d_test(NAME TestOutputOutput DATA cow.vtp ARGS --reference=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutput.png DEPENDS TestOutput NO_BASELINE)
f3d_test(NAME TestUnsupportedInputOutput DATA unsupportedFile.dummy REGEXP "No files loaded, no rendering performed" NO_BASELINE)
f3d_test(NAME TestOutputNoBackground DATA cow.vtp ARGS --no-background NO_BASELINE)

# Basic record and play test
f3d_test(NAME TestInteractionRecord DATA cow.vtp ARGS --interaction-test-record=${CMAKE_BINARY_DIR}/Testing/Temporary/TestInteractionRecord.log NO_BASELINE)
f3d_test(NAME TestInteractionPlay DATA cow.vtp ARGS --interaction-test-play=${CMAKE_BINARY_DIR}/Testing/Temporary/TestInteractionRecord.log DEPENDS TestInteractionRecord NO_BASELINE)

# Command Script Test
f3d_test(NAME TestCommandScriptBasic DATA dragon.vtu SCRIPT TestCommandScriptBasic.txt) # roll_camera 90;toggle ui.scalar_bar;print_scene_info;increase_light_intensity
f3d_test(NAME TestCommandScriptInvalidCommand DATA dragon.vtu SCRIPT TestCommandScriptInvalid.txt REGEXP "Command: \"INVALID_COMMAND_1\" is not recognized, ignoring" NO_BASELINE) # INVALID_COMMAND_1
f3d_test(NAME TestCommandScriptMissingFile DATA dragon.vtu SCRIPT TestCommandScriptMissingFile.txt REGEXP "Unable to open command script file" NO_BASELINE)
f3d_test(NAME TestCommandScriptPrintScene DATA dragon.vtu SCRIPT TestCommandScriptPrintScene.txt REGEXP "Camera position: 2.23745,3.83305,507.598" NO_BASELINE) # print_scene_info
f3d_test(NAME TestCommandScriptPrintColoring DATA dragon.vtu SCRIPT TestCommandScriptPrintColoring.txt REGEXP "Not coloring" NO_BASELINE) # print_coloring_info
f3d_test(NAME TestCommandScriptPrintMesh DATA dragon.vtu SCRIPT TestCommandScriptPrintMesh.txt REGEXP "Number of points: 13268" NO_BASELINE) # print_mesh_info
f3d_test(NAME TestCommandScriptPrintOptions DATA dragon.vtu SCRIPT TestCommandScriptPrintOptions.txt REGEXP "interactor.invert_zoom: false" NO_BASELINE) # print_options_info
f3d_test(NAME TestCommandScriptAlias DATA dragon.vtu SCRIPT TestCommandScriptAlias.txt --reference=${F3D_SOURCE_DIR}/testing/baselines/TestCommandScriptAlias.png) # alias myrotate roll_camera 90;myrotate
f3d_test(NAME TestCommandScriptReset DATA dragon.vtu suzanne.stl ARGS --edges SCRIPT TestCommandScriptReset.txt) # reset render.show_edges; load_next_file_group;
f3d_test(NAME TestParseOptionalBoolExtraArg DATA dragon.vtu SCRIPT TestParseOptionalBoolExtraArg.txt REGEXP "Command: load_previous_file_group takes at most 1 argument, got 2 arguments instead." NO_BASELINE) # load_previous_file_group true extra
f3d_test(NAME TestRemoveFileGroups DATA dragon.vtu SCRIPT TestRemoveFileGroups.txt NO_DATA_FORCE_RENDER UI) # remove_file_groups
f3d_test(NAME TestCommandScriptInvalidReaderOptions DATA dragon.vtu SCRIPT TestCommandScriptInvalidReaderOptions.txt REGEXP "point to an inexistent option, ignoring" NO_BASELINE) # set_reader_option invalid value

# Window position test
f3d_test(NAME TestPosition DATA dragon.vtu ARGS --position=100,100 NO_BASELINE)
f3d_test(NAME TestInvalidPosition DATA dragon.vtu ARGS --position=100,100,300 REGEXP "Provided position could not be applied" NO_BASELINE)
f3d_test(NAME TestInvalidResolution DATA dragon.vtu RESOLUTION 800 REGEXP "Provided resolution could not be applied" NO_BASELINE)

# Simple verbosity test
f3d_test(NAME TestVerbose DATA dragon.vtu ARGS --verbose REGEXP "Number of points: 13268\nNumber of cells: 26532" NO_BASELINE)

# Test verbose quiet
f3d_test(NAME TestVerboseQuiet DATA mb.vtm ARGS --verbose=quiet REGEXP_FAIL "A non data set block was ignored while reading a multiblock." NO_BASELINE)

# Test verbose error
f3d_test(NAME TestVerboseError DATA mb.vtm ARGS --verbose=error REGEXP_FAIL "A non data set block was ignored while reading a multiblock." NO_BASELINE)

# Test verbose warning, rely on the log::info about image comparison
f3d_test(NAME TestVerboseWarning DATA cow.vtp ARGS --verbose=warning REGEXP_FAIL "Image comparison success with an error difference of")

# Test verbose debug
f3d_test(NAME TestVerboseDebug DATA dragon.vtu ARGS --verbose REGEXP "Number of points: 13268\nNumber of cells: 26532" NO_BASELINE)

# Test verbose invalid verbose value
f3d_test(NAME TestVerboseInvalid DATA dragon.vtu ARGS --verbose=invalid REGEXP "Unrecognized verbose level" NO_BASELINE)

# Unknown scalar array verbosity test
f3d_test(NAME TestVerboseWrongArray DATA dragon.vtu ARGS -s --coloring-array=dummy --verbose REGEXP "Unknown scalar array: \"dummy\"" NO_BASELINE)

# Default scalar array verbosity test
f3d_test(NAME TestVerboseDefaultScalar DATA HeadMRVolume.mhd ARGS -s --verbose REGEXP "Coloring using point array named MetaImage, Magnitude" NO_BASELINE)

# Volume array verbosity test
f3d_test(NAME TestVerboseVolume DATA HeadMRVolume.mhd ARGS -v --verbose REGEXP "Coloring using point array named MetaImage .forced., Magnitude" NO_BASELINE)

# Incorrect component test
f3d_test(NAME TestIncorrectComponent DATA dragon.vtu ARGS -s --coloring-component=4 REGEXP "Invalid component index: 4" NO_BASELINE)

# Incorrect volume coloring with multi file
f3d_test(NAME TestIncorrectMultiFileVolume DATA multi ARGS -sv --coloring-array=Normals --multi-file-mode=all REGEXP "Cannot find the array \"Normals\" to display volume with" NO_BASELINE)

# Incorrect color map
f3d_test(NAME TestIncorrectColormap DATA IM-0001-1983.dcm ARGS --scalar-coloring --roughness=1 --colormap=0,1,0,0,1,0,1 REGEXP "Incorrect number of tokens in provided colormap" NO_BASELINE)

# Test opening a directory
f3d_test(NAME TestVerboseDirectory DATA mb REGEXP "mb_0_0.vtu" NO_RENDER)

# Test opening multiple file and rendering only one
f3d_test(NAME TestVerboseMultiFileRender DATA mb REGEXP "An output image was saved using a single 3D file, other provided 3D files were ignored." NO_BASELINE)
f3d_test(NAME TestVerboseMultiFileCompare DATA mb REGEXP "Image comparison was performed using a single 3D file, other provided 3D files were ignored.")

# Test Animation invalid code paths
f3d_test(NAME TestVerboseAnimationIndexError2 DATA cow.vtp ARGS --animation-index=1 --verbose REGEXP "An animation index has been specified but there are no animation available." NO_BASELINE)
f3d_test(NAME TestVerboseAnimationNoAnimationTime DATA cow.vtp ARGS --animation-time=2 --verbose REGEXP "No animation available, cannot load a specific animation time" NO_BASELINE)

# Test Grid verbose output
f3d_test(NAME TestVerboseGrid DATA suzanne.ply ARGS -g --verbose REGEXP "Grid origin set to" NO_BASELINE)

# Test bounding box no render output
f3d_test(NAME TestNoRenderBBox DATA suzanne.ply NO_RENDER REGEXP "Scene bounding box: -1.32819,1.32819,-0.971822,0.939236,-0.778266,0.822441")

# Test Scalars coloring verbose output
f3d_test(NAME TestVerboseScalars DATA suzanne.ply ARGS -s --verbose REGEXP "Coloring using point array named Normals, Magnitude." NO_BASELINE)

# Test direct scalars surface rendering with a 9 comp array
f3d_test(NAME TestTensorsDirect DATA tensors.vti ARGS -s --coloring-array=tensors1 --coloring-component=-2 REGEXP "Direct scalars rendering not supported by array with more than 4 components" NO_BASELINE)

# Test direct scalars volume rendering with a 9 comp array
f3d_test(NAME TestTensorsVolumeDirect DATA tensors.vti ARGS -v -s --coloring-array=tensors1 --coloring-component=-2 REGEXP "Direct scalars rendering not supported by array with more than 4 components" NO_BASELINE)

# Test volume rendering without any array
f3d_test(NAME TestVerboseVolumeNoArray DATA cow.vtp ARGS -v REGEXP "Cannot use volume with this data" NO_BASELINE)

# Test scalar rendering without any array
f3d_test(NAME TestVerboseNoArray DATA cow.vtp ARGS -s --verbose=debug REGEXP "No array to color with" NO_BASELINE)

# Test invalid scalar range
f3d_test(NAME TestInvalidScalarsRange DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1 --coloring-range=0,1,2 REGEXP "Invalid scalar range provided, using automatic range" NO_BASELINE)

# Test invalid backface type
f3d_test(NAME TestInvalidBackface DATA backface.vtp ARGS --backface-type=invalid REGEXP "is not a valid backface type, assuming it is not set" NO_BASELINE)

# Test non existent file, do not create nonExistentFile.vtp
f3d_test(NAME TestVerboseNonExistentFile DATA nonExistentFile.vtp REGEXP "File .*nonExistentFile.vtp does not exist" NO_RENDER)

# Test non existent font file, do not create nonExistentFile.ttf
f3d_test(NAME TestVerboseNonExistentFont DATA suzanne.ply ARGS -n --font-file=${F3D_SOURCE_DIR}/testing/data/nonExistentFile.ttf REGEXP "Cannot find \".*nonExistentFile.ttf\" font file" NO_BASELINE)

# Test scalar rendering without any array
f3d_test(NAME TestVerboseOptionsCLI ARGS -x --verbose=debug REGEXP "'axis' = '1' from CLI options" NO_BASELINE)
f3d_test(NAME TestVerboseOptionsConfig ARGS --verbose=debug CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json REGEXP "'ui.axis' = 'true'" NO_BASELINE)

# Test quiet with a non existent file
f3d_test(NAME TestQuietNonExistentFile DATA nonExistentFile.vtp ARGS --verbose=quiet --no-render REGEXP_FAIL "File .*nonExistentFile.vtp does not exist")

# Test non supported file, do not add support for .dummy file.
f3d_test(NAME TestUnsupportedFileText DATA unsupportedFile.dummy ARGS --filename REGEXP ".*unsupportedFile.dummy is not a file of a supported file format" NO_RENDER)

# Test non existent texture, do not add a dummy.png
f3d_test(NAME TestNonExistentTexture DATA cow.vtp ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/dummy.png REGEXP "Texture file does not exist" NO_BASELINE)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  # Test invalid file
  f3d_test(NAME TestInvalidFile DATA duck_invalid.gltf REGEXP "failed to load scene" NO_BASELINE)

  # Test invalid animation
  f3d_test(NAME TestAnimationInvalid DATA BoxAnimated_invalid_animation.gltf ARGS --animation-time 1 REGEXP "Could not load time value: 1" NO_BASELINE)
endif ()

# Test invalid texture
f3d_test(NAME TestInvalidTexture DATA cow.vtp ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/invalid.png REGEXP "Cannot open texture file" NO_BASELINE)

# Test invalid color
f3d_test(NAME TestInvalidColor DATA cow.vtp ARGS --color=0,0,0,1 REGEXP "Provided vector does not have the right size" NO_BASELINE)

# Test non existent interaction file, do not add a TestNonExistentInteraction
f3d_test(NAME TestNonExistentInteraction DATA cow.vtp INTERACTION REGEXP "Interaction record file to play does not exist" NO_BASELINE)

# Test unknown options, do not add a --colour option
f3d_test(NAME TestUnknownOptionVerbose ARGS --colour=a=b REGEXP "Did you mean '--color=a=b'?")
f3d_test(NAME TestUnknownOptionExitCode ARGS --colour=1,0,0 WILL_FAIL)
f3d_test(NAME TestUnknownAppOptionVerbose ARGS --revolution=17,32 REGEXP "Did you mean '--resolution=17,32'?")
f3d_test(NAME TestUnknownBooleanOptionVerbose ARGS --helpo REGEXP "Did you mean '--help'?")
f3d_test(NAME TestUnknownInputOptionVerbose ARGS --inprut=file REGEXP "Did you mean '--input=file'?")

# Test app option error code path
f3d_test(NAME TestAppOptionError ARGS --animation-time=invalid REGEXP "Could not parse 'invalid' into 'animation-time' option")
f3d_test(NAME TestAppOptionNoConfigError ARGS --no-config=invalid CONFIG invalid REGEXP "Could not parse 'invalid' into 'no-config' option, assuming false")

# Test non-existent config filepath, do not add a dummy.json
f3d_test(NAME TestNonExistentConfigFilePath DATA cow.vtp CONFIG "${F3D_SOURCE_DIR}/testing/configs/dummy.json" REGEXP "Configuration file does not exist" NO_BASELINE)

# Test non-existent config filename, do not add a dummy.json or dummy.d
f3d_test(NAME TestNonExistentConfigFileStem DATA cow.vtp CONFIG "dummy" REGEXP "Configuration file for \"dummy\" could not be found" NO_BASELINE)

# Test invalid config file
f3d_test(NAME TestInvalidConfigFile DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/invalid.json REGEXP "Unable to parse the configuration file" NO_BASELINE)

# Test invalid reset key in config file
f3d_test(NAME TestInvalidResetOptions DATA cow.vtp ARGS --reset= REGEXP "Invalid option: 'reset' must be followed by a valid option name, ignoring entry" NO_BASELINE)

# Test invalid multifile mode
f3d_test(NAME TestInvalidMultiFileMode DATA mb/recursive ARGS --multi-file-mode=add REGEXP "Unrecognized multi-file-mode: add. Assuming \"single\" mode." NO_BASELINE)

# Test unnamed cameras/animation
f3d_test(NAME TestVerboseUnnamedCamera DATA Cameras.gltf ARGS --verbose REGEXP "1: unnamed_1" NO_BASELINE)
f3d_test(NAME TestVerboseUnnamedAnimation DATA BoxAnimated.gltf ARGS --verbose REGEXP "0: unnamed_0" NO_BASELINE)

# Test invalid value in config file
f3d_test(NAME TestConfigFileInvalidOptions DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/invalid_options.json REGEXP "Error processing config file" NO_BASELINE)

# Test invalid value in config file
f3d_test(NAME TestConfigFileNoOptions DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/no_options.json REGEXP "does not contains options" NO_BASELINE)

# Test update interaction verbose
f3d_test(NAME TestConfigFileBindingsVerbose DATA dragon.vtu ARGS --verbose CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json REGEXP "'Shift.O' : '`toggle model.point_sprites.enable` '" NO_BASELINE)

# Test list-bindings display with config file
f3d_test(NAME TestConfigFileBindingsList ARGS --list-bindings CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json REGEXP "Ctrl.Shift.O `toggle ui.filename`" NO_BASELINE)
f3d_test(NAME TestConfigFileBindingsListData DATA dragon.vtu ARGS --list-bindings CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json REGEXP "Any.3        `roll_camera 90`" NO_BASELINE)

# Test invalid value in config file
f3d_test(NAME TestConfigFileInvalidValue DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/invalid_value.json REGEXP "must be a string, a boolean or a number" NO_BASELINE)

# Test non-parsable value in config file
f3d_test(NAME TestConfigFileNonParsableValue DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/nonparsable_value.json REGEXP "Cannot parse nonparsable into an int" NO_BASELINE)

# Test inexistent key in config file
f3d_test(NAME TestConfigFileInexistentKey DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/inexistent_key.json REGEXP "does not exists , did you mean 'scene.animation.index" NO_BASELINE)

# Test quiet in config file
f3d_test(NAME TestConfigFileQuiet DATA nonExistentFile.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/quiet.json REGEXP_FAIL "File .*/testing/data/nonExistentFile.vtp does not exist" NO_BASELINE)

# Test no file with config file
f3d_test(NAME TestNoFileConfigFile CONFIG ${F3D_SOURCE_DIR}/testing/configs/verbose.json ARGS --verbose REGEXP "No files to load provided" NO_BASELINE)

# Test that --no-config overrides --config
f3d_test(NAME TestNoConfigWithConfig DATA f3d.glb ARGS --no-config CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json)

# Test that config file search display logs
f3d_test(NAME TestConfigFileLocationVerbose ARGS --verbose CONFIG inexistent.json REGEXP "Candidate config file not found: .*inexistent.json" NO_BASELINE)

# Test invalid match type in config file
f3d_test(NAME TestConfigInvalidMatchType DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/invalid_match_type.json REGEXP "There was an error in the config .*invalid_match_type.json for regex pattern.*" NO_BASELINE)

# Test glob match type but no match expression in config file
f3d_test(NAME TestConfigMatchTypeNoMatch DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/match_type_no_match.json REGEXP "A config block in config file .*match_type_no_match.json has match-type glob but no match expression, using a catch-all regex" NO_BASELINE)

# Test invalid glob expression in config file
f3d_test(NAME TestConfigInvalidGlobExp DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/invalid_glob_exp.json REGEXP "There was an error in the config .*invalid_glob_exp.json for glob pattern.*" NO_BASELINE)

# Test glob matching
if(WIN32)
  set(GLOBSTAR_EXPR [=[**\\\\*stl]=])
else()
  set(GLOBSTAR_EXPR "**/*stl")
endif()
configure_file("${F3D_SOURCE_DIR}/testing/configs/glob.json.in" "${CMAKE_BINARY_DIR}/glob.json")
f3d_test(NAME TestMatchFirstConfigGlobOptionBlock DATA dragon.vtu CONFIG ${CMAKE_BINARY_DIR}/glob.json)
f3d_test(NAME TestMatchSecondConfigGlobOptionBlock DATA suzanne.stl CONFIG ${CMAKE_BINARY_DIR}/glob.json)

# Test exact matching
cmake_path(SET FIRST_EXACT_PATH "${F3D_SOURCE_DIR}/testing/data/suzanne.obj")
cmake_path(NATIVE_PATH FIRST_EXACT_PATH NATIVE_FIRST_EXACT_PATH)
cmake_path(SET SECOND_EXACT_PATH "${F3D_SOURCE_DIR}/testing/data/suzanne.ply")
cmake_path(NATIVE_PATH SECOND_EXACT_PATH NATIVE_SECOND_EXACT_PATH)
if(WIN32)
  string(REPLACE "\\" "\\\\" NATIVE_FIRST_EXACT_PATH "${NATIVE_FIRST_EXACT_PATH}")
  string(REPLACE "\\" "\\\\" NATIVE_SECOND_EXACT_PATH "${NATIVE_SECOND_EXACT_PATH}")
endif()
configure_file("${F3D_SOURCE_DIR}/testing/configs/exact.json.in" "${CMAKE_BINARY_DIR}/exact.json")
f3d_test(NAME TestExactMatch DATA suzanne.obj CONFIG ${CMAKE_BINARY_DIR}/exact.json)

# Test help display
f3d_test(NAME TestHelp ARGS --help REGEXP "Usage:")
f3d_test(NAME TestHelpPositional ARGS --help REGEXP "file1 file2 \.\.\.")

# Test version display
f3d_test(NAME TestVersion ARGS --version REGEXP "Version:")

# Test list-readers display
f3d_test(NAME TestReadersList ARGS --list-readers REGEXP_FAIL "No registered reader found")

# Test invalid component string coverage
f3d_test(NAME TestInteractionInvalidComponent INTERACTION UI DATA cow.vtp ARGS --coloring-component=1 NO_BASELINE) #H

# Test multi plugin list-readers
if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestReadersListMultiplePlugins ARGS --list-readers --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load")
endif()

# Test bindings-list display
f3d_test(NAME TestBindingsList ARGS --list-bindings REGEXP "Any.5        Toggle Orthographic Projection")

# Test rendering backends
# For some reason the sanitizer detects leaks because of EGL and OSMesa
f3d_test(NAME TestRenderingBackendAuto DATA cow.vtp RENDERING_BACKEND auto)
if(UNIX AND NOT APPLE AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914 AND F3D_SANITIZER STREQUAL "none")
  if(F3D_TESTING_ENABLE_EGL_TESTS)
    f3d_test(NAME TestRenderingBackendEGL DATA cow.vtp RENDERING_BACKEND egl)
    f3d_test(NAME TestRenderingBackendEGLCheckClass DATA cow.vtp RENDERING_BACKEND egl ARGS --verbose REGEXP "vtkF3DEGLRenderWindow" NO_BASELINE)
    f3d_test(NAME TestRenderingBackendEGLNoInteractiveVerbose RENDERING_BACKEND egl REGEXP "You are using an offscreen configuration, interactive rendering is disabled" NO_BASELINE NO_OUTPUT)
  endif()

  if(F3D_TESTING_ENABLE_OSMESA_TESTS)
    f3d_test(NAME TestRenderingBackendOSMesa DATA cow.vtp RENDERING_BACKEND osmesa)
    f3d_test(NAME TestRenderingBackendOSMesaCheckClass DATA cow.vtp RENDERING_BACKEND osmesa ARGS --verbose REGEXP "vtkOSOpenGLRenderWindow" NO_BASELINE)
    f3d_test(NAME TestRenderingBackendOSMesaNoInteractiveVerbose RENDERING_BACKEND osmesa REGEXP "You are using an offscreen configuration, interactive rendering is disabled" NO_BASELINE NO_OUTPUT)
  endif()

  if(F3D_TESTING_ENABLE_GLX_TESTS)
    f3d_test(NAME TestRenderingBackendGLX DATA cow.vtp RENDERING_BACKEND glx)
    f3d_test(NAME TestRenderingBackendGLXNoDisplay DATA cow.vtp RENDERING_BACKEND glx ENV "DISPLAY=:7" REGEXP "Cannot create a window" NO_BASELINE)
    f3d_test(NAME TestRenderingBackendGLXCheckClass DATA cow.vtp RENDERING_BACKEND glx ARGS --verbose REGEXP "vtkF3DGLXRenderWindow" NO_BASELINE)
    f3d_test(NAME TestRenderingBackendLinuxAutoCheckClass DATA cow.vtp ARGS --verbose REGEXP "vtkF3DGLXRenderWindow" NO_BASELINE)
  endif()

  f3d_test(NAME TestRenderingBackendWGLFailure DATA cow.vtp RENDERING_BACKEND wgl REGEXP "Cannot use a WGL context on this platform" NO_BASELINE)
endif()
if(WIN32)
  f3d_test(NAME TestRenderingBackendWGL DATA cow.vtp RENDERING_BACKEND wgl)
  f3d_test(NAME TestRenderingBackendWGLCheckClass DATA cow.vtp RENDERING_BACKEND wgl ARGS --verbose REGEXP "vtkF3DWGLRenderWindow" NO_BASELINE)
  f3d_test(NAME TestRenderingBackendWindowsAutoCheckClass DATA cow.vtp ARGS --verbose REGEXP "vtkF3DWGLRenderWindow" NO_BASELINE)
  f3d_test(NAME TestRenderingBackendGLXFailure DATA cow.vtp RENDERING_BACKEND glx REGEXP "Cannot use a GLX context on this platform" NO_BASELINE)
endif()
f3d_test(NAME TestRenderingBackendInvalid DATA cow.vtp RENDERING_BACKEND invalid ARGS --verbose REGEXP "rendering-backend value is invalid, falling back to" NO_BASELINE)

# Test that f3d can try to read a system config file
add_test(NAME f3d::TestNoNoConfig COMMAND $<TARGET_FILE:f3d> --no-render)
set_tests_properties(f3d::TestNoNoConfig PROPERTIES TIMEOUT 4)

# Test that we can try loading a plugin without F3D_PLUGINS_PATH being defined
add_test(NAME f3d::TestNoEnvInvalidPlugin COMMAND $<TARGET_FILE:f3d> --load-plugins=invalid --no-render)
set_tests_properties(f3d::TestNoEnvInvalidPlugin PROPERTIES PASS_REGULAR_EXPRESSION "Cannot open the library")

# Test invalid CLI args
add_test(NAME f3d::TestInvalidCLIArgs COMMAND $<TARGET_FILE:f3d> --up)
set_tests_properties(f3d::TestInvalidCLIArgs PROPERTIES PASS_REGULAR_EXPRESSION "Error parsing command line arguments")

# Test that f3d resolution can be controlled from config file
add_test(NAME f3d::TestConfigResolution COMMAND $<TARGET_FILE:f3d> --config=${F3D_SOURCE_DIR}/testing/configs/resolution.json ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestConfigResolution.png --reference=${F3D_SOURCE_DIR}/testing/baselines/TestConfigResolution.png)

# Test filename template with multiple files
add_test(NAME f3d::TestMultiFileFileNameTemplate COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl ${F3D_SOURCE_DIR}/testing/data/dragon.vtu --output=${CMAKE_BINARY_DIR}/Testing/Temporary/{model.ext}.png --multi-file-mode=all --verbose)
set_tests_properties(f3d::TestMultiFileFileNameTemplate PROPERTIES PASS_REGULAR_EXPRESSION "multi_file.png")

# Test filename template with no files
add_test(NAME f3d::TestNoFileFileNameTemplate COMMAND $<TARGET_FILE:f3d> --output=${CMAKE_BINARY_DIR}/Testing/Temporary/{model.ext}.png --verbose)
set_tests_properties(f3d::TestNoFileFileNameTemplate PROPERTIES PASS_REGULAR_EXPRESSION "no_file.png")
set_tests_properties(f3d::TestNoFileFileNameTemplate PROPERTIES ENVIRONMENT "CTEST_F3D_NO_DATA_FORCE_RENDER=1")

if(NOT WIN32)
  f3d_test(NAME TestInvalidCache ENV "HOME=" REGEXP "Could not use default cache directory" NO_BASELINE)

  add_test(NAME f3d::TestHOMEOutput COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=~/Testing/Temporary/TestHOMEOutput.png --resolution=300,300 --no-config)
  set_tests_properties(f3d::TestHOMEOutput PROPERTIES ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}")
  set_tests_properties(f3d::TestHOMEOutput PROPERTIES FIXTURES_SETUP f3d::TestHOMEOutput_FIXTURE)

  add_test(NAME f3d::TestHOMEReference COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestHOMEReference.png --reference=~/Testing/Temporary/TestHOMEOutput.png --resolution=300,300 --no-config)
  set_tests_properties(f3d::TestHOMEReference PROPERTIES ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}")
  set_tests_properties(f3d::TestHOMEReference PROPERTIES FIXTURES_REQUIRED f3d::TestHOMEOutput_FIXTURE)

  add_test(NAME f3d::TestHOMEOutputTemplate COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=~/Testing/Temporary/{model}{n}.png --resolution=300,300 --verbose)
  set_tests_properties(f3d::TestHOMEOutputTemplate PROPERTIES ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}")

  f3d_test(NAME TestHOMEInput ARGS --input=~/testing/data/suzanne.stl ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_test(NAME TestHOMEConfig DATA suzanne.stl CONFIG ~/testing/configs/complex.json ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_test(NAME TestHOMEPlugin ARGS --load-plugins=~/testing/data/invalid.so REGEXP "Cannot open the library" NO_BASELINE ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_test(NAME TestHOMEInteractionRecord DATA cow.vtp ARGS --interaction-test-record=~/Testing/Temporary/TestHOMEInteractionRecord.log NO_BASELINE ENV "HOME=${CMAKE_BINARY_DIR}")
  f3d_test(NAME TestHOMEInteractionPlay DATA cow.vtp ARGS --interaction-test-play=~/Testing/Temporary/TestHOMEInteractionRecord.log DEPENDS TestHOMEInteractionRecord NO_BASELINE ENV "HOME=${CMAKE_BINARY_DIR}")
  f3d_test(NAME TestHOMEColorMapFile DATA dragon.vtu ARGS --colormap-file=~/testing/data/viridis8.png --scalar-coloring --coloring-component=1 ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_test(NAME TestHOMEScreenshot DATA suzanne.ply ARGS --screenshot-filename=~/Testing/Temporary/TestHOMEScreenshot.png --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log REGEXP "saving screenshot to ${CMAKE_BINARY_DIR}/Testing/Temporary/TestHOMEScreenshot.png" NO_BASELINE ENV "HOME=${CMAKE_BINARY_DIR}")
  f3d_test(NAME TestHOMECommandScript DATA suzanne.ply ARGS --command-script=~/testing/scripts/TestHOMECommandScript.txt REGEXP "Camera focal point" NO_BASELINE ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_test(NAME TestHOMEFontFile DATA suzanne.stl ARGS -n --font-file=~/testing/data/Crosterian.ttf ENV "HOME=${F3D_SOURCE_DIR}" UI)
  f3d_test(NAME TestHOMETexture DATA suzanne.ply ARGS --texture-matcap=~/testing/data/skin.png ENV "HOME=${F3D_SOURCE_DIR}")

  if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20221220)
    f3d_test(NAME TestHOMEHDRI DATA suzanne.stl ARGS --hdri-file=~/testing/data/palermo_park_1k.hdr --hdri-ambient --hdri-skybox ENV "HOME=${F3D_SOURCE_DIR}" LONG_TIMEOUT)
    f3d_test(NAME TestHOMEInteractionDropHDRICollapse INTERACTION_CONFIGURE ENV "HOME=${F3D_SOURCE_DIR}" LONG_TIMEOUT)#X;DropEvent dragon.vtu;DropEvent palermo.hdr;
  endif()

  STRING(RANDOM LENGTH 257 ALPHABET "x" _f3d_test_invalid_folder)
  f3d_test(NAME TestColorMapTooLong DATA dragon.vtu ARGS --colormap-file=${_f3d_test_invalid_folder}/file.ext --scalar-coloring REGEXP "File name too long" NO_BASELINE)
  f3d_test(NAME TestScreenshotTooLong DATA suzanne.ply ARGS --screenshot-filename=${_f3d_test_invalid_folder}/file.ext --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log REGEXP "File name too long" NO_BASELINE)
  f3d_test(NAME TestInputTooLong ARGS --input=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE)
  f3d_test(NAME TestReferenceTooLong DATA suzanne.ply ARGS --output=file.png --reference=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputTooLong DATA suzanne.ply ARGS --output=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputWithReferenceTooLong DATA suzanne.ply ARGS --reference=file.png --output=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputWithExistingReferenceTooLong DATA suzanne.ply ARGS --reference=${F3D_SOURCE_DIR}/testing/data/world.png --output=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)

  if(NOT APPLE)
    f3d_test(NAME TestXDG_CONFIG_HOME DATA suzanne.ply CONFIG config_build REGEXP "Using config directory ${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d" NO_RENDER NO_BASELINE ENV "XDG_CONFIG_HOME=${CMAKE_BINARY_DIR}/share")
    f3d_test(NAME TestXDG_CACHE_HOMECoverage DATA suzanne.ply NO_RENDER NO_BASELINE ENV "XDG_CACHE_HOME=${CMAKE_BINARY_DIR}")

    # Setting XDG_CACHE_HOME is needed for cache to work when HOME is not set
    f3d_test(NAME TestNoHOMEScreenshot DATA suzanne.ply ARGS --screenshot-filename=TestNoHOMEScreenshot.png --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log  REGEXP "saving screenshot to ${CMAKE_BINARY_DIR}/application/testing/TestNoHOMEScreenshot.png" NO_BASELINE ENV "XDG_CACHE_HOME=${CMAKE_BINARY_DIR};HOME=")
    f3d_test(NAME TestNoHOMEConfig DATA suzanne.ply CONFIG config_build REGEXP "Using config directory ${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d" NO_RENDER NO_BASELINE ENV "XDG_CACHE_HOME=${CMAKE_BINARY_DIR};HOME=")
  endif()

else()
  cmake_path(SET _user_profile_path ${CMAKE_BINARY_DIR}/Testing/Temporary/ùser/🥷)
  cmake_path(NATIVE_PATH _user_profile_path _user_profile_path)
  add_test(NAME f3d::TestSetupWinUserProfile COMMAND ${CMAKE_COMMAND} -E make_directory "${_user_profile_path}/AppData/Local")
  f3d_test(NAME TestWinUserProfileNonStandard DATA dragon.vtu ENV "USERPROFILE=${_user_profile_path}" DEPENDS TestSetupWinUserProfile)
endif()

# Test failure without a reference, please do not create a TestNoRef.png file
f3d_test(NAME TestNoRef DATA cow.vtp WILL_FAIL)

# Test failure without a reference and without an output, please do not create a TestNoRef.png file
f3d_test(NAME TestNoRefNoOutput DATA cow.vtp ARGS --reference=${F3D_SOURCE_DIR}/testing/baselines/TestNoRef.png REGEXP "use the output option to output current rendering into an image file." NO_BASELINE NO_OUTPUT)

# Test failure with a bad reference, please do not create a good TestBadRef.png file
f3d_test(NAME TestBadRef DATA cow.vtp WILL_FAIL)

# Test failure with a bad reference without an output, please do not create a good TestBadRef.png file
f3d_test(NAME TestBadRefNoOutput DATA cow.vtp ARGS --reference=${F3D_SOURCE_DIR}/testing/baselines/TestBadRef.png REGEXP "Use the --output option to be able to output current rendering and diff images into files." NO_BASELINE NO_OUTPUT)

# Test failure with a bad interaction play file, please do not create a dummy.log
f3d_test(NAME TestPlayNoFile DATA cow.vtp ARGS --interaction-test-play=${CMAKE_BINARY_DIR}/Testing/Temporary/dummy.log WILL_FAIL)

# Test that --help is displayed even when there is an unknown option
f3d_test(NAME TestHelpPrecedenceWithUnknownOption ARGS --help --unknown REGEXP "Usage:"  NO_BASELINE)

# Test that --version is displayed even when there is an unknown option
f3d_test(NAME TestVersionPrecedenceWithUnknownOption ARGS --version --unknown REGEXP "Version:" NO_BASELINE)

# Test rendering backend list
if(WIN32)
  f3d_test(NAME TestRenderingBackenListWGL ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "wgl: available")
elseif(APPLE)
  f3d_test(NAME TestRenderingBackenListCOCOA ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "cocoa: available")
endif()

if(F3D_TESTING_ENABLE_GLX_TESTS AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914)
  f3d_test(NAME TestRenderingBackenListGLX ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "glx: available")
endif()

if(F3D_TESTING_ENABLE_EGL_TESTS AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914)
  f3d_test(NAME TestRenderingBackenListEGL ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "egl: available")
endif()

if(F3D_TESTING_ENABLE_OSMESA_TESTS)
  f3d_test(NAME TestRenderingBackenListOSMesa ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "osmesa: available")
endif()

# Test scan plugins
if(NOT F3D_MACOS_BUNDLE)
  f3d_test(NAME TestScanPluginsCheckNative ARGS --scan-plugins NO_RENDER NO_BASELINE REGEXP " - native")

  if(F3D_PLUGIN_BUILD_HDF)
    f3d_test(NAME TestScanPluginsCheckHDF ARGS --scan-plugins NO_RENDER NO_BASELINE REGEXP " - hdf")
  endif()
endif()

# Watch testing require GLX rendering on Linux
# SIGTERM is not caught with VTK 9.2.6 for some reason
if(F3D_MODULE_DMON AND (WIN32 OR APPLE OR (F3D_TESTING_ENABLE_GLX_TESTS AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)))
  if(UNIX)
    set(_f3d_os_script_ext "sh")
    set(_f3d_os_script_exec "")
  elseif(WIN32)
    set(_f3d_os_script_ext "ps1")
    find_program(_f3d_os_script_exec NAMES pwsh powershell)
  endif()
  # Custom bash/pwsh test for testing watch option
  add_test (NAME f3d::TestWatch COMMAND ${_f3d_os_script_exec} ${CMAKE_CURRENT_SOURCE_DIR}/test_watch.${_f3d_os_script_ext} $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data ${CMAKE_BINARY_DIR}/Testing/Temporary)
  set_tests_properties(f3d::TestWatch PROPERTIES RUN_SERIAL TRUE TIMEOUT 30)
endif()
