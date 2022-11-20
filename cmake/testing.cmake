# F3D Testing
function(f3d_test)

  cmake_parse_arguments(F3D_TEST "DEFAULT_LIGHTS;LONG_TIMEOUT;HDRI;INTERACTION;NO_BASELINE;NO_RENDER;NO_OUTPUT;WILL_FAIL" "NAME;CONFIG;DATA;RESOLUTION;THRESHOLD;REGEXP;REGEXP_FAIL;DEPENDS" "ARGS" ${ARGN})

  if(F3D_TEST_CONFIG)
    list(APPEND F3D_TEST_ARGS "--config=${F3D_TEST_CONFIG}")
  else()
    list(APPEND F3D_TEST_ARGS "--dry-run")
  endif()

  if (F3D_TEST_DATA)
    list(APPEND F3D_TEST_ARGS "${CMAKE_SOURCE_DIR}/testing/data/${F3D_TEST_DATA}")
  endif()

  if(F3D_TEST_INTERACTION)
    list(APPEND F3D_TEST_ARGS "--interaction-test-play=${CMAKE_SOURCE_DIR}/testing/recordings/${F3D_TEST_NAME}.log")
  endif()

  if(F3D_TEST_NO_RENDER)
    list(APPEND F3D_TEST_ARGS "--no-render")
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
    if(NOT (F3D_TEST_DEFAULT_LIGHTS AND F3D_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON))
      list(APPEND F3D_TEST_ARGS "--ref=${CMAKE_SOURCE_DIR}/testing/baselines/${F3D_TEST_NAME}.png")

      if(F3D_TEST_THRESHOLD)
        list(APPEND F3D_TEST_ARGS "--ref-threshold=${F3D_TEST_THRESHOLD}")
      endif()
    endif()
  endif()

  add_test(NAME "f3d::${F3D_TEST_NAME}" COMMAND $<TARGET_FILE:f3d> ${F3D_TEST_ARGS} COMMAND_EXPAND_LISTS)

  set(_timeout "30")
  if(F3D_TEST_LONG_TIMEOUT OR F3D_TEST_INTERACTION OR F3D_TEST_HDRI)
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

  if(NOT F3D_ENABLE_LONG_TIMEOUT_TESTS)
    if(F3D_TEST_LONG_TIMEOUT OR F3D_TEST_INTERACTION OR F3D_TEST_HDRI)
      set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES DISABLED ON)
    endif()
  elseif(NOT F3D_ENABLE_HDRI_TESTS)
    if(F3D_TEST_HDRI)
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
    set_tests_properties(f3d::${F3D_TEST_DEPENDS} PROPERTIES FIXTURES_SETUP f3d::${F3D_TEST_DEPENDS}_FIXTURE)
    set_tests_properties(f3d::${F3D_TEST_NAME} PROPERTIES FIXTURES_REQUIRED f3d::${F3D_TEST_DEPENDS}_FIXTURE)
  endif()

endfunction()

f3d_test(NAME TestPLY DATA suzanne.ply DEFAULT_LIGHTS)
f3d_test(NAME TestOBJ DATA suzanne.obj ARGS --geometry-only DEFAULT_LIGHTS)
f3d_test(NAME TestSTL DATA suzanne.stl DEFAULT_LIGHTS)
f3d_test(NAME TestVTU DATA dragon.vtu DEFAULT_LIGHTS)
f3d_test(NAME TestVTP DATA cow.vtp DEFAULT_LIGHTS)
f3d_test(NAME TestVTR DATA RectGrid2.vtr ARGS --scalars --roughness=1 DEFAULT_LIGHTS)
f3d_test(NAME TestVTS DATA bluntfin.vts DEFAULT_LIGHTS)
f3d_test(NAME TestVTM DATA mb.vtm DEFAULT_LIGHTS)
f3d_test(NAME TestVTK DATA cow.vtk DEFAULT_LIGHTS)
f3d_test(NAME TestNRRD DATA beach.nrrd ARGS -s DEFAULT_LIGHTS)
f3d_test(NAME TestGridX DATA suzanne.ply ARGS -g --up=+X DEFAULT_LIGHTS)
f3d_test(NAME TestGridY DATA suzanne.ply ARGS -g --up=+Y DEFAULT_LIGHTS)
f3d_test(NAME TestGridZ DATA suzanne.ply ARGS -g --up=+Z DEFAULT_LIGHTS)
f3d_test(NAME TestAxis DATA suzanne.ply ARGS -x DEFAULT_LIGHTS)
f3d_test(NAME TestPointCloud DATA pointsCloud.vtp ARGS -o --point-size=20)
f3d_test(NAME TestPointCloudBar DATA pointsCloud.vtp ARGS -sob --point-size=20)
f3d_test(NAME TestPointCloudUG DATA pointsCloud.vtu ARGS -o --point-size=20)
f3d_test(NAME TestPointCloudVolume DATA bluntfin.vts ARGS -sob)
f3d_test(NAME TestPointCloudDefaultScene DATA pointsCloud.vtp ARGS --point-size=20)
f3d_test(NAME TestVRMLImporter DATA bot2.wrl)
f3d_test(NAME Test3DSImporter DATA iflamigm.3ds ARGS --up=+Z)
f3d_test(NAME TestScalars DATA suzanne.ply ARGS --scalars=Normals --comp=1 DEFAULT_LIGHTS)
f3d_test(NAME TestScalarsRange DATA suzanne.ply ARGS --scalars=Normals --comp=1 --range=0,1 DEFAULT_LIGHTS)
f3d_test(NAME TestScalarsWithBar DATA suzanne.ply ARGS -b --scalars=Normals --comp=0 DEFAULT_LIGHTS)
f3d_test(NAME TestGLTFImporter DATA WaterBottle.glb)
f3d_test(NAME TestGLTFImporterWithAnimation DATA BoxAnimated.gltf DEFAULT_LIGHTS) # Technically not a default lights tests, but fails for some reason on vtk 9.0.0
f3d_test(NAME TestGLTFSkin DATA SimpleSkin.gltf)
f3d_test(NAME TestGLTFReaderWithAnimation DATA BoxAnimated.gltf ARGS --geometry-only DEFAULT_LIGHTS)
f3d_test(NAME TestDicom DATA IM-0001-1983.dcm ARGS --scalars --roughness=1 DEFAULT_LIGHTS)
f3d_test(NAME TestMHD DATA HeadMRVolume.mhd ARGS --scalars --roughness=1 DEFAULT_LIGHTS)
f3d_test(NAME TestVTICell DATA waveletMaterial.vti ARGS --scalars=Material -c --roughness=1 DEFAULT_LIGHTS)
f3d_test(NAME TestSSAO LONG_TIMEOUT DATA suzanne.ply ARGS -q DEFAULT_LIGHTS)
f3d_test(NAME TestDepthPeeling DATA suzanne.ply ARGS -sp --opacity=0.9 DEFAULT_LIGHTS)
f3d_test(NAME TestBackground DATA suzanne.ply ARGS --bg-color=0.8,0.2,0.9 THRESHOLD 100 DEFAULT_LIGHTS)
f3d_test(NAME TestGridWithDepthPeeling DATA suzanne.ply ARGS -gp --opacity 0.2)
f3d_test(NAME TestFilename DATA suzanne.ply ARGS -n DEFAULT_LIGHTS)
f3d_test(NAME TestFilenameWhiteBg DATA suzanne.ply ARGS -n --bg-color=1,1,1 DEFAULT_LIGHTS)
f3d_test(NAME TestCityGML DATA Part-4-Buildings-V4-one.gml DEFAULT_LIGHTS)
f3d_test(NAME TestPTS DATA samplePTS.pts)
f3d_test(NAME TestColormap DATA IM-0001-1983.dcm ARGS --scalars --roughness=1 --colormap=0,1,0,0,1,0,1,0 DEFAULT_LIGHTS)
f3d_test(NAME TestCameraConfiguration DATA suzanne.obj ARGS --camera-position=0,0,-10 -x --camera-view-up=1,0,0 --camera-focal-point=1,0,0 --camera-view-angle=20 --camera-azimuth-angle=40 --camera-elevation-angle=-80)
f3d_test(NAME TestCameraClipping DATA checkerboard_colorful.obj CONFIG ${CMAKE_SOURCE_DIR}/testing/data/checkerboard_colorful.json RESOLUTION 800,600)
f3d_test(NAME TestToneMapping DATA suzanne.ply ARGS -t DEFAULT_LIGHTS)
f3d_test(NAME TestDepthPeelingToneMapping DATA suzanne.ply ARGS --opacity=0.9 -pt DEFAULT_LIGHTS)
f3d_test(NAME TestDefaultConfigFile DATA dragon.vtu CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json DEFAULT_LIGHTS)
f3d_test(NAME TestDefaultConfigFileAnotherBlock DATA vase_4comp.vti CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json DEFAULT_LIGHTS)
f3d_test(NAME TestDefaultConfigFileUp DATA suzanne.stl CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json DEFAULT_LIGHTS)
f3d_test(NAME TestDefaultConfigFileAndCommand DATA suzanne.stl ARGS --up=-Y CONFIG ${CMAKE_SOURCE_DIR}/resources/config.json DEFAULT_LIGHTS)
f3d_test(NAME TestVolume DATA HeadMRVolume.mhd ARGS -v --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 LONG_TIMEOUT THRESHOLD 300) # High threshold for volume as it is dependent on the OpenGL implementation
f3d_test(NAME TestVolumeInverse DATA HeadMRVolume.mhd ARGS -vi --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 LONG_TIMEOUT)
f3d_test(NAME TestVolumeMag DATA vase_4comp.vti ARGS -vb LONG_TIMEOUT)
f3d_test(NAME TestVolumeComp DATA vase_4comp.vti ARGS -vb --comp=3 LONG_TIMEOUT)
f3d_test(NAME TestVolumeDirect DATA vase_4comp.vti ARGS -vb --comp=-2 LONG_TIMEOUT)
f3d_test(NAME TestVolumeCells DATA waveletArrays.vti ARGS -vb --cells LONG_TIMEOUT)
f3d_test(NAME TestVolumeNonScalars DATA waveletArrays.vti ARGS -vb --scalars=RandomPointScalars LONG_TIMEOUT)
f3d_test(NAME TestTextureNormal DATA WaterBottle.glb ARGS --geometry-only --texture-normal=${CMAKE_SOURCE_DIR}/testing/data/normal.png --normal-scale=0.1 DEFAULT_LIGHTS)
f3d_test(NAME TestTextureMaterial DATA WaterBottle.glb ARGS --geometry-only --texture-material=${CMAKE_SOURCE_DIR}/testing/data/red_mod.jpg --roughness=1 --metallic=1 DEFAULT_LIGHTS)
f3d_test(NAME TestTextureMaterialWithOptions DATA WaterBottle.glb ARGS --geometry-only --texture-material=${CMAKE_SOURCE_DIR}/testing/data/red_mod.jpg --roughness=0.5 --metallic=0.5 DEFAULT_LIGHTS)
f3d_test(NAME TestTextureColor DATA WaterBottle.glb ARGS --geometry-only --texture-base-color=${CMAKE_SOURCE_DIR}/testing/data/albedo_mod.png --depth-peeling DEFAULT_LIGHTS)
f3d_test(NAME TestTextureEmissive DATA WaterBottle.glb ARGS --geometry-only --texture-emissive=${CMAKE_SOURCE_DIR}/testing/data/red.jpg --emissive-factor=0.1,0.1,0.1 DEFAULT_LIGHTS)
f3d_test(NAME TestTextures DATA WaterBottle.glb ARGS --geometry-only --texture-material=${CMAKE_SOURCE_DIR}/testing/data/red.jpg --roughness=1 --metallic=1 --texture-base-color=${CMAKE_SOURCE_DIR}/testing/data/albedo.png --texture-normal=${CMAKE_SOURCE_DIR}/testing/data/normal.png --texture-emissive=${CMAKE_SOURCE_DIR}/testing/data/red.jpg --emissive-factor=0.1,0.1,0.1 DEFAULT_LIGHTS)
f3d_test(NAME TestMetaDataImporter DATA BoxAnimated.gltf ARGS -m DEFAULT_LIGHTS)
f3d_test(NAME TestMultiblockMetaData DATA mb.vtm ARGS -m DEFAULT_LIGHTS)
f3d_test(NAME TestTIFF DATA logo.tif ARGS -sy --up=-Y DEFAULT_LIGHTS)
f3d_test(NAME TestLightIntensityBrighter DATA cow.vtp ARGS --light-intensity=5.0 DEFAULT_LIGHTS)
f3d_test(NAME TestLightIntensityDarker DATA cow.vtp ARGS --light-intensity=0.2 DEFAULT_LIGHTS)
f3d_test(NAME TestLightIntensityBrighterFullScene DATA WaterBottle.glb ARGS --light-intensity=5.0 DEFAULT_LIGHTS)
f3d_test(NAME TestLightIntensityDarkerFullScene DATA WaterBottle.glb ARGS --light-intensity=0.2 DEFAULT_LIGHTS)
f3d_test(NAME TestUTF8 DATA "(ノಠ益ಠ )ノ.vtp" DEFAULT_LIGHTS)
f3d_test(NAME TestFont DATA suzanne.ply ARGS -n --font-file=${CMAKE_SOURCE_DIR}/testing/data/AttackGraffiti-3zRBM.ttf DEFAULT_LIGHTS)
f3d_test(NAME TestAnimationIndex DATA InterpolationTest.glb ARGS --animation-index=7 DEFAULT_LIGHTS)
f3d_test(NAME TestMaxSizeBelow DATA suzanne.stl ARGS --max-size=1 DEFAULT_LIGHTS)
f3d_test(NAME TestMaxSizeAbove DATA WaterBottle.glb ARGS --max-size=1 REGEXP "No file loaded, file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestNonExistentFile DATA nonExistentFile.vtp ARGS --filename WILL_FAIL)
f3d_test(NAME TestUnsupportedFile DATA unsupportedFile.dummy ARGS --filename WILL_FAIL)
f3d_test(NAME TestVerboseLoadPlugin ARGS --verbose REGEXP "Loading plugin \"native\"" NO_BASELINE)
f3d_test(NAME TestNonExistentPlugin ARGS --load-plugins=dummy REGEXP "Plugin failed to load" NO_BASELINE)

# color texture with opacity needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9467
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220811)
  f3d_test(NAME TestTextureColorWithOptions DATA WaterBottle.glb ARGS --geometry-only --texture-base-color=${CMAKE_SOURCE_DIR}/testing/data/albedo_mod.png --color=1,1,0 --opacity=0.4 --depth-peeling DEFAULT_LIGHTS)
endif()

# Tests that do not work with VTK 9.0.1 and have been
# fixed prior to the date based versioning system
if(VTK_VERSION VERSION_GREATER 9.0.1)
  f3d_test(NAME TestOBJImporter DATA world.obj)
  f3d_test(NAME TestGLTFImporterUnlit DATA UnlitTest.glb)
  f3d_test(NAME TestMaterial DATA suzanne.ply ARGS --color=0.72,0.45,0.2 --metallic=0.7 --roughness=0.2)
  f3d_test(NAME TestMetaData DATA pdiag.vtu ARGS -m)
  f3d_test(NAME TestEdges DATA suzanne.ply ARGS -e)
  f3d_test(NAME TestLineWidth DATA cow.vtk ARGS -e --line-width=5)
  f3d_test(NAME TestLineWidthFullScene DATA suzanne.obj ARGS -e --line-width=3 --up=-Y)
  f3d_test(NAME TestPointCloudFullScene DATA pointsCloud.gltf ARGS --point-size=20)

  # Test Verbose animation, no baseline needed
  f3d_test(NAME TestVerboseAnimation DATA InterpolationTest.glb ARGS --verbose NO_BASELINE REGEXP "7: CubicSpline Translation")
  # Test Animation index out of domain error
  f3d_test(NAME TestVerboseAnimationIndexError1 DATA InterpolationTest.glb ARGS --animation-index=48 NO_BASELINE REGEXP "Specified animation index is greater than the highest possible animation index, enabling the first animation.")
  f3d_test(NAME TestInteractionAnimation DATA InterpolationTest.glb ARGS --animation-index=-1 INTERACTION)#Space;Space;
  f3d_test(NAME TestInteractionAnimationMovement DATA KameraAnim.glb ARGS --camera-index=1 INTERACTION)#Space;MouseMovement;Space;
  # Test exit hotkey
  f3d_test(NAME TestInteractionSimpleExit DATA cow.vtp REGEXP "Interactor has been stopped" INTERACTION NO_BASELINE) #Escape;
  if(NOT F3D_MODULE_RAYTRACING) # TODO fix this once we have OSPray in CI
    f3d_test(NAME TestInteractionCheatsheet DATA cow.vtp INTERACTION) #H
    f3d_test(NAME TestInteractionCheatsheetScalars DATA dragon.vtu ARGS --scalars --comp=-2 INTERACTION) #HSSS
  endif()
endif()

# Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
if(VTK_VERSION VERSION_GREATER_EQUAL 9.0.20210303)
  f3d_test(NAME TestCameraPersp DATA Cameras.gltf ARGS --camera-index=0)
  f3d_test(NAME TestCameraOrtho DATA Cameras.gltf ARGS --camera-index=1)
  f3d_test(NAME TestCameraIndexConfiguration DATA Cameras.gltf ARGS --camera-index=0  --camera-azimuth-angle=15 --camera-position=0.7,0.5,3)
  # Test Verbose camera
  f3d_test(NAME TestVerboseCamera DATA Cameras.gltf ARGS --camera-index=1 --verbose NO_RENDER REGEXP "0:.*1:")
endif()

# Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7900
if(VTK_VERSION VERSION_GREATER_EQUAL 9.0.20210429)
  f3d_test(NAME TestGLTFMorph DATA SimpleMorph.gltf)
endif()

# no-background test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8501
if(VTK_VERSION VERSION_GREATER_EQUAL 9.1.20211007)
  f3d_test(NAME TestNoBackground DATA cow.vtp ARGS --no-background)
  f3d_test(NAME TestThumbnailConfigFile DATA dragon.vtu CONFIG ${CMAKE_SOURCE_DIR}/resources/thumbnail.json DEFAULT_LIGHTS)
  f3d_test(NAME TestThumbnailConfigFileAnotherBlock DATA vase_4comp.vti CONFIG ${CMAKE_SOURCE_DIR}/resources/thumbnail.json DEFAULT_LIGHTS)
  f3d_test(NAME TestThumbnailConfigFileUp DATA suzanne.stl CONFIG ${CMAKE_SOURCE_DIR}/resources/thumbnail.json DEFAULT_LIGHTS)
endif()

# HDRI test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8825
if(VTK_VERSION VERSION_GREATER_EQUAL 9.1.20220117)
  f3d_test(NAME TestHDRI HDRI DATA suzanne.ply ARGS --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr DEFAULT_LIGHTS)
  f3d_test(NAME TestHDRIBlur HDRI DATA suzanne.ply ARGS -u --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr DEFAULT_LIGHTS)
  f3d_test(NAME TestHDRIBlurRatio HDRI DATA suzanne.ply RESOLUTION 600,100 ARGS -u --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr DEFAULT_LIGHTS)
  f3d_test(NAME TestHDRIEdges HDRI DATA suzanne.ply ARGS -e --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr DEFAULT_LIGHTS)
  f3d_test(NAME TestHDRI8Bit DATA suzanne.ply ARGS --hdri=${CMAKE_SOURCE_DIR}/testing/data/logo.tif HDRI)
  f3d_test(NAME TestHDRIOrient DATA suzanne.stl ARGS --up=+Z --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr HDRI)
  f3d_test(NAME TestHDRIToneMapping DATA suzanne.ply ARGS -t --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr HDRI)
  f3d_test(NAME TestInteractionHDRIMove DATA suzanne.ply ARGS --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr HDRI INTERACTION) #Shift+MouseRight;
  f3d_test(NAME TestInteractionHDRIBlur DATA suzanne.ply ARGS --hdri=${CMAKE_SOURCE_DIR}/testing/data/palermo_park_1k.hdr INTERACTION HDRI DEFAULT_LIGHTS) #U
endif()

if(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestOSPRayGLTF DATA WaterBottle.glb ARGS -r --samples=1)
  f3d_test(NAME TestOSPRayBackground DATA suzanne.ply ARGS -r --samples=1 --bg-color=1,0,0)
  f3d_test(NAME TestOSPRayPointCloud DATA pointsCloud.vtp ARGS -r --point-size=20)
  f3d_test(NAME TestOSPRayDenoise DATA suzanne.ply ARGS -rd --samples=1)
  f3d_test(NAME TestVersionRaytracing ARGS --version REGEXP "Raytracing module: ON")
  f3d_test(NAME TestInteractionOSPRayDenoise DATA suzanne.ply ARGS --samples=1 INTERACTION) #RD
else(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestInteractionOSPRayDenoiseNoRaytracing DATA suzanne.ply ARGS INTERACTION NO_BASELINE REGEXP "Raytracing options can't be used if F3D has not been built with raytracing") #RD
endif()

if(F3D_MODULE_EXTERNAL_RENDERING)
  f3d_test(NAME TestVersionExternal ARGS --version REGEXP "External rendering module: ON")
endif()

if(F3D_PLUGIN_BUILD_EXODUS)
  f3d_test(NAME TestExodus DATA disk_out_ref.ex2 ARGS --load-plugins=exodus -s --camera-position=-11,-2,-49 DEFAULT_LIGHTS)
  f3d_test(NAME TestGenericImporterAnimation DATA small.ex2 DEFAULT_LIGHTS ARGS --load-plugins=exodus)
  # Test Generic Importer Verbose animation
  f3d_test(NAME TestVerboseGenericImporterAnimation DATA small.ex2 ARGS --load-plugins=exodus --verbose NO_BASELINE REGEXP "0: default")

  # Test animation with generic importer
  f3d_test(NAME TestInteractionAnimationGenericImporter DATA small.ex2 INTERACTION NO_BASELINE ARGS --load-plugins=exodus)#Space;Space;
endif()

if(F3D_PLUGIN_BUILD_OCCT)
  f3d_test(NAME TestSTEP DATA cube.stp DEFAULT_LIGHTS ARGS --load-plugins=occt)
  f3d_test(NAME TestIGES DATA spacer.igs DEFAULT_LIGHTS ARGS --load-plugins=occt)
endif()

if(F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestOFF DATA teapot.off ARGS --up=+Z --load-plugins=assimp)
  f3d_test(NAME TestDXF DATA PinkEggFromLW.dxf ARGS --bg-color=1,1,1 -p --load-plugins=assimp)
  f3d_test(NAME TestFBX DATA phong_cube.fbx ARGS --load-plugins=assimp)

  f3d_test(NAME TestVerboseCameraAssimp DATA duck.dae ARGS --verbose --load-plugins=assimp NO_BASELINE REGEXP "camera1")

  # TGA Reader needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8224
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.0.20210805) # for TGA support and embedded textures
    f3d_test(NAME TestVerboseAssimp DATA robot_kyle_walking.fbx ARGS --verbose --load-plugins=assimp NO_BASELINE REGEXP "Robot2")
    f3d_test(NAME TestDAE DATA duck.dae ARGS --load-plugins=assimp)

    # Embedded texture are only working with assimp 5.1.X
    if("${F3D_ASSIMP_VERSION}" VERSION_GREATER_EQUAL "5.1.0")
      f3d_test(NAME TestTexturedFBX DATA slime.fbx ARGS --load-plugins=assimp)
    endif()
  endif()

  # Animation are working only with assimp 5.0.0 and 5.0.1.
  # Both report a assimp version 5.0.0 when using find_package
  if("${F3D_ASSIMP_VERSION}" VERSION_EQUAL "5.0.0")
    f3d_test(NAME TestInteractionFBXAnim DATA robot_kyle_walking.fbx ARGS --load-plugins=assimp INTERACTION) #Space;Space;
  endif()
endif()

if(F3D_PLUGIN_BUILD_ALEMBIC)
  f3d_test(NAME TestABC DATA suzanne.abc ARGS --load-plugins=alembic)
endif()

if(F3D_PLUGIN_BUILD_ALEMBIC AND F3D_PLUGIN_BUILD_ASSIMP)
  f3d_test(NAME TestMultiplePluginsLoad DATA cow.vtp ARGS --load-plugins=assimp,alembic NO_BASELINE REGEXP_FAIL "Plugin failed to load")
endif()

## Interaction Tests
# Test hotkeys
f3d_test(NAME TestInteractionPostFX DATA cow.vtp INTERACTION DEFAULT_LIGHTS) #PQAT
f3d_test(NAME TestInteractionActors DATA cow.vtp INTERACTION DEFAULT_LIGHTS) #EXGMN
f3d_test(NAME TestInteractionTimer DATA cow.vtp NO_BASELINE INTERACTION) #Z
f3d_test(NAME TestInteractionMisc DATA cow.vtp NO_BASELINE INTERACTION) #KK
f3d_test(NAME TestInteractionCycleCell DATA waveletArrays.vti INTERACTION) #VCCC
f3d_test(NAME TestInteractionCycleComp DATA dragon.vtu INTERACTION DEFAULT_LIGHTS) #SYYYY
f3d_test(NAME TestInteractionCycleScalars DATA dragon.vtu INTERACTION DEFAULT_LIGHTS) #BSSSS
f3d_test(NAME TestInteractionVolumeInverse DATA HeadMRVolume.mhd ARGS --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 INTERACTION) #VI
f3d_test(NAME TestInteractionPointCloud DATA pointsCloud.vtp ARGS --point-size=20 INTERACTION) #O
f3d_test(NAME TestInteractionDirectory DATA mb INTERACTION ARGS --scalars DEFAULT_LIGHTS) #Right;Right;Right;Left;Up;
f3d_test(NAME TestInteractionDirectoryLoop DATA mb INTERACTION ARGS --scalars DEFAULT_LIGHTS) #Left;Left;Left;
f3d_test(NAME TestInteractionAnimationNotStopped DATA InterpolationTest.glb NO_BASELINE INTERACTION)#Space;Space;
f3d_test(NAME TestInteractionResetCamera DATA dragon.vtu INTERACTION DEFAULT_LIGHTS)#MouseMovements;Return;
f3d_test(NAME TestInteractionTensorsCycleComp DATA tensors.vti ARGS --scalars --comp=-2  INTERACTION DEFAULT_LIGHTS) #SYYYYYYYYYY
f3d_test(NAME TestInteractionCycleScalarsCompCheck DATA dragon.vtu ARGS -b --scalars --comp=2 INTERACTION DEFAULT_LIGHTS) #S
f3d_test(NAME TestInteractionConfigFileMulti DATA multi CONFIG ${CMAKE_SOURCE_DIR}/testing/configs/complex.json INTERACTION DEFAULT_LIGHTS) #XG;Right;N;Right;Right
f3d_test(NAME TestInteractionConfigFileAndCommand DATA multi ARGS -o CONFIG ${CMAKE_SOURCE_DIR}/testing/configs/complex.json INTERACTION DEFAULT_LIGHTS) #O;Left
f3d_test(NAME TestInteractionDumpSceneState DATA dragon.vtu NO_BASELINE INTERACTION REGEXP "Camera position: 2.26745,3.82625,507.698")#?
f3d_test(NAME TestInteractionCycleVerbose DATA dragon.vtu ARGS --verbose -s NO_BASELINE INTERACTION REGEXP "Not coloring")#SSSSYC
f3d_test(NAME TestInteractionEmptyDrop INTERACTION REGEXP "Drop event without any provided files.")#DropEvent Empty;
f3d_test(NAME TestInteractionCameraUpdate DATA dragon.vtu INTERACTION DEFAULT_LIGHTS) #MouseWheel;MouseWheel;MouseWheel;S
 
# Drop file test needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9199
if(VTK_VERSION VERSION_GREATER_EQUAL 9.1.20220519) # Drop file test uses stream version 1.2

  configure_file("${CMAKE_SOURCE_DIR}/testing/recordings/TestInteractionDropFiles.log.in"
    "${CMAKE_BINARY_DIR}/TestInteractionDropFiles.log")

  f3d_test(NAME TestInteractionDropFiles ARGS -x "--interaction-test-play=${CMAKE_BINARY_DIR}/TestInteractionDropFiles.log")#DropEvent cow.vtp;#DropEvent dragon.vtu suzanne.stl;
endif()

## Tests to increase coverage
# Output option test
f3d_test(NAME TestOutput DATA cow.vtp NO_BASELINE)
f3d_test(NAME TestOutputOutput DATA cow.vtp ARGS --ref=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutput.png DEPENDS TestOutput NO_BASELINE)
f3d_test(NAME TestUnsupportedInputOutput DATA unsupportedFile.dummy REGEXP "No file loaded, no rendering performed" NO_BASELINE)
f3d_test(NAME TestOutputNoBackground DATA cow.vtp ARGS --no-background NO_BASELINE)

# Basic record and play test
f3d_test(NAME TestInteractionRecord DATA cow.vtp ARGS --interaction-test-record=${CMAKE_BINARY_DIR}/Testing/Temporary/interaction.log NO_BASELINE)
f3d_test(NAME TestInteractionPlay DATA cow.vtp ARGS --interaction-test-play=${CMAKE_BINARY_DIR}/Testing/Temporary/interaction.log DEPENDS TestInteractionRecord NO_BASELINE)

# Window position test
f3d_test(NAME TestPosition DATA dragon.vtu ARGS --position=100,100 NO_BASELINE)

# Simple verbosity test
f3d_test(NAME TestVerbose DATA dragon.vtu ARGS -s --verbose REGEXP "Number of points: 69827\nNumber of cells: 139650" NO_RENDER)

# Unknown scalar array verbosity test
f3d_test(NAME TestVerboseWrongArray DATA dragon.vtu ARGS --scalars=dummy --verbose REGEXP "Unknown scalar array: dummy" NO_BASELINE)

# Default scalar array verbosity test
f3d_test(NAME TestVerboseDefaultScalar DATA HeadMRVolume.mhd ARGS -s --verbose REGEXP "Coloring using point array named MetaImage, Magnitude" NO_BASELINE)

# Incorrect component test
f3d_test(NAME TestIncorrectComponent DATA dragon.vtu ARGS -s --comp=4 REGEXP "Invalid component index: 4" NO_BASELINE)

# Incorrect color map
f3d_test(NAME TestIncorrectColormap DATA IM-0001-1983.dcm ARGS --scalars --roughness=1 --colormap=0,1,0,0,1,0,1 REGEXP "Specified color map list count is not a multiple of 4, ignoring it." NO_BASELINE)

# Test opening a directory
f3d_test(NAME TestDirectory DATA mb REGEXP "Loading: .*mb_._0.vt." NO_RENDER)

# Test Animation invalid index
f3d_test(NAME TestVerboseAnimationIndexError2 DATA cow.vtp ARGS --animation-index=1 --verbose REGEXP "An animation index has been specified but there are no animation available." NO_BASELINE)

# Test Grid verbose output
f3d_test(NAME TestVerboseGrid DATA suzanne.ply ARGS -g --verbose REGEXP "Grid origin set to" NO_BASELINE)

# Test Scalars coloring verbose output
f3d_test(NAME TestVerboseScalars DATA suzanne.ply ARGS -s --verbose REGEXP "Coloring using point array named Normals, Magnitude." NO_BASELINE)

# Test direct scalars surface rendering with a 9 comp array
f3d_test(NAME TestTensorsDirect DATA tensors.vti ARGS --scalars=tensors1 --comp=-2 REGEXP "Direct scalars rendering not supported by array with more than 4 components" NO_BASELINE)

# Test direct scalars volume rendering with a 9 comp array
f3d_test(NAME TestTensorsVolumeDirect DATA tensors.vti ARGS -v --scalars=tensors1 --comp=-2 REGEXP "Direct scalars rendering not supported by array with more than 4 components" NO_BASELINE)

# Test volume rendering without any array
f3d_test(NAME TestVolumeNoArray DATA cow.vtp ARGS -v REGEXP "No array to color with" NO_BASELINE)

# Test non existent file, do not create nonExistentFile.vtp
f3d_test(NAME TestVerboseNonExistentFile DATA nonExistentFile.vtp ARGS --filename --verbose REGEXP "File .*/testing/data/nonExistentFile.vtp does not exist" NO_RENDER)

# Test non existent font file, do not create nonExistentFile.ttf
f3d_test(NAME TestVerboseNonExistentFont DATA suzanne.ply ARGS -n --font-file=${CMAKE_SOURCE_DIR}/testing/data/nonExistentFile.ttf REGEXP "Cannot find \".*/testing/data/nonExistentFile.ttf\" font file" NO_BASELINE)

# Test non existent file, do not create nonExistentFile.vtp
f3d_test(NAME TestQuietNonExistentFile DATA nonExistentFile.vtp ARGS --filename --verbose --quiet REGEXP_FAIL "File .*/testing/data/nonExistentFile.vtp does not exist" NO_RENDER)

# Test non supported file, do not add support for .dummy file.
f3d_test(NAME TestUnsupportedFileText DATA unsupportedFile.dummy ARGS --filename REGEXP ".*/testing/data/unsupportedFile.dummy is not a file of a supported file format" NO_RENDER)

# Test non existent texture, do not add a dummy.png
f3d_test(NAME TestNonExistentTexture DATA cow.vtp ARGS --texture-material=${CMAKE_SOURCE_DIR}/testing/data/dummy.png REGEXP "Texture file does not exist" NO_BASELINE)

# Test invalid texture
f3d_test(NAME TestInvalidTexture DATA cow.vtp ARGS --texture-material=${CMAKE_SOURCE_DIR}/testing/data/invalid.png REGEXP "Cannot open texture file" NO_BASELINE)

# Test non existent HDRI, do not add a dummy.png
f3d_test(NAME TestNonExistentHDRI DATA cow.vtp ARGS --hdri=${CMAKE_SOURCE_DIR}/testing/data/dummy.png REGEXP "HDRI file does not exist" NO_BASELINE)

# Test non existent interaction file, do not add a TestNonExistentInteraction
f3d_test(NAME TestNonExistentInteraction DATA cow.vtp INTERACTION REGEXP "Interaction record file to play does not exist" NO_BASELINE)

# Test invalid provided HDRI
f3d_test(NAME TestInvalidHDRI DATA cow.vtp ARGS --hdri=${CMAKE_SOURCE_DIR}/testing/data/invalid.png REGEXP "Cannot open HDRI file" NO_BASELINE)

# Test invalid options, do not add a --dummy option
f3d_test(NAME TestInvalidOption ARGS --dummy REGEXP "Error parsing options:")

# Test non-existent config filepath, do not add a dummy.json
f3d_test(NAME TestNonExistentConfigFilePath DATA cow.vtp CONFIG "${CMAKE_SOURCE_DIR}/testing/configs/dummy.json" REGEXP "Configuration file does not exist" NO_BASELINE)

# Test non-existent config filename, do not add a dummy.json
f3d_test(NAME TestNonExistentConfigFilename DATA cow.vtp CONFIG "dummy.json" REGEXP "Configuration file for \"dummy.json\" could not been found" NO_BASELINE)

# Test non-existent config filename, do not add a dummy.json
f3d_test(NAME TestNonExistentConfigFileStem DATA cow.vtp CONFIG "dummy" REGEXP "Configuration file for \"dummy\" could not been found" NO_BASELINE)

# Test invalid config file
f3d_test(NAME TestInvalidConfigFile DATA cow.vtp CONFIG ${CMAKE_SOURCE_DIR}/testing/configs/invalid.json REGEXP "Unable to parse the configuration file" NO_BASELINE)

# Test quiet in config file
f3d_test(NAME TestConfigFileQuiet DATA nonExistentFile.vtp CONFIG ${CMAKE_SOURCE_DIR}/testing/configs/quiet.json REGEXP_FAIL "File .*/testing/data/nonExistentFile.vtp does not exist" NO_RENDER)

# Test help display
f3d_test(NAME TestHelp ARGS --help REGEXP "Usage:")

# Test version display
f3d_test(NAME TestVersion ARGS --version REGEXP "Version:")

# Test readers-list display
f3d_test(NAME TestReadersList ARGS --readers-list REGEXP_FAIL "No registered reader found")

# Test that f3d can try to read a system config file
add_test(NAME f3d::TestNoDryRun COMMAND $<TARGET_FILE:f3d> --no-render)
set_tests_properties(f3d::TestNoDryRun PROPERTIES TIMEOUT 2)

# Test failure without a reference, please do not create a TestNoRef.png file
f3d_test(NAME TestNoRef DATA cow.vtp WILL_FAIL)

# Test failure without a reference and without an output, please do not create a TestNoRef.png file
f3d_test(NAME TestNoRefNoOutput DATA cow.vtp ARGS --ref ${CMAKE_SOURCE_DIR}/testing/baselines/TestNoRef.png REGEXP "use the output option to output current rendering into an image file." NO_BASELINE NO_OUTPUT)

# Test failure with a bad reference, please do not create a good TestBadRef.png file
f3d_test(NAME TestBadRef DATA cow.vtp WILL_FAIL)

# Test failure with a bad reference without an output, please do not create a good TestBadRef.png file
f3d_test(NAME TestBadRefNoOutput DATA cow.vtp ARGS --ref ${CMAKE_SOURCE_DIR}/testing/baselines/TestBadRef.png REGEXP "Use the --output option to be able to output current rendering and diff images into files." NO_BASELINE NO_OUTPUT)

# Test failure with a bad interaction play file, please do not create a dummy.log
f3d_test(NAME TestPlayNoFile DATA cow.vtp ARGS --interaction-test-play=${CMAKE_BINARY_DIR}/Testing/Temporary/dummy.log WILL_FAIL)
