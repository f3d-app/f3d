## Tests the many features of F3D
## Must ONLY depends on native plugin

## Grid
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
f3d_test(NAME TestGridWithDepthPeeling DATA suzanne.ply ARGS -gp --opacity=0.2)

# Backface
f3d_test(NAME TestBackfaceVisible DATA backface.vtp ARGS --backface-type=visible)
f3d_test(NAME TestBackfaceHidden DATA backface.vtp ARGS --backface-type=hidden)

# Point cloud
f3d_test(NAME TestPointCloud DATA pointsCloud.vtp ARGS -o --point-sprites-size=20)
f3d_test(NAME TestPointCloudBar DATA pointsCloud.vtp ARGS -sob --point-sprites-size=20)
f3d_test(NAME TestPointCloudUG DATA pointsCloud.vtu ARGS -o --point-sprites-size=20)
f3d_test(NAME TestPointCloudVolume DATA bluntfin.vts ARGS -sob)
f3d_test(NAME TestPointCloudDefaultScene DATA pointsCloud.vtp ARGS --point-size=20)

# https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12678
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251206)
  f3d_test(NAME TestPointCloudFullScene DATA pointsCloud.gltf ARGS --point-size=20)
endif()

# FX
f3d_test(NAME TestSSAO DATA suzanne.ply ARGS -q)
f3d_test(NAME TestDepthPeeling DATA suzanne.ply ARGS -sp --opacity=0.9)
f3d_test(NAME TestToneMapping DATA suzanne.ply ARGS -t TONE_MAPPING)
f3d_test(NAME TestDepthPeelingToneMapping DATA suzanne.ply ARGS --opacity=0.9 -pt TONE_MAPPING)
f3d_test(NAME TestBackground DATA suzanne.ply ARGS --background-color=0.8,0.2,0.9 THRESHOLD 0.1) # Threshold is needed for legacy image comparison for VTK 9.3
f3d_test(NAME TestNoBackground DATA cow.vtp ARGS --no-background)
f3d_test(NAME TestMaterial DATA suzanne.ply ARGS --color=0.72,0.45,0.2 --metallic=0.7 --roughness=0.2)
f3d_test(NAME TestMaterialFullScene DATA WaterBottle.glb ARGS --color=0.9,0.1,0.1 --metallic=0.7 --roughness=0.2)
f3d_test(NAME TestMinBaseIOR DATA suzanne.ply ARGS --base-ior=1.0)
f3d_test(NAME TestMaxBaseIOR DATA suzanne.ply ARGS --base-ior=2.5)
f3d_test(NAME TestEdges DATA suzanne.ply ARGS -e)
f3d_test(NAME TestLineWidth DATA cow.vtk ARGS -e --line-width=5)
f3d_test(NAME TestLineWidthFullScene DATA suzanne.obj ARGS -e --line-width=3 --up=-Y)

# Scalar coloring
f3d_test(NAME TestScalars DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1)
f3d_test(NAME TestScalarsCell DATA f3d.vtp ARGS --scalar-coloring --coloring-by-cells --coloring-component=-2 --up=+Z)
f3d_test(NAME TestScalarsRange DATA suzanne.ply ARGS -s --coloring-array=Normals --coloring-component=1 --coloring-range=0,1)
f3d_test(NAME TestScalarsWithBar DATA suzanne.ply ARGS -b -s --coloring-array=Normals --coloring-component=0)
f3d_test(NAME TestScalarsWithBarContinuity DATA suzanne.ply ARGS -b -s --coloring-array=Normals --coloring-component=0)
f3d_test(NAME TestComponentName DATA from_abq.vtu ARGS --scalar-coloring --coloring-scalar-bar --coloring-component=2)
f3d_test(NAME TestUserMatrixColoring DATA InterpolationTest.glb ARGS --scalar-coloring --coloring-array=TEXCOORD_0)
f3d_test(NAME TestSkinColoring DATA SimpleSkin.gltf ARGS --scalar-coloring --coloring-array=WEIGHTS_0)
f3d_test(NAME TestMorphColoring DATA SimpleMorph.gltf ARGS --scalar-coloring)
f3d_test(NAME TestAnimationUserMatrixColoring DATA InterpolationTest.glb ARGS --scalar-coloring --coloring-array=TEXCOORD_0 --animation-time=0.5)
f3d_test(NAME TestAnimationSkinColoring DATA SimpleSkin.gltf ARGS --scalar-coloring --coloring-array=WEIGHTS_0 --animation-time=2)
f3d_test(NAME TestAnimationMorphColoring DATA SimpleMorph.gltf ARGS --scalar-coloring --animation-time=0.5)
f3d_test(NAME TestAnimationInputChangeColoring DATA v_rock2.mdl ARGS --scalar-coloring --animation-time=0.01 --animation-indices=1)

# Depth
f3d_test(NAME TestDisplayDepth DATA dragon.vtu ARGS --display-depth)
f3d_test(NAME TestDisplayDepthColorMap DATA dragon.vtu ARGS --display-depth --scalar-coloring=True)
f3d_test(NAME TestDisplayDepthCustomColorMap DATA dragon.vtu ARGS --display-depth --scalar-coloring --colormap=0,red,1,blue)

# DPI scaling
f3d_test(NAME TestDPI125 DATA dragon.vtu ARGS -nm --dpi-aware DPI_SCALE 1.25 UI)
f3d_test(NAME TestDPI150 DATA dragon.vtu ARGS -nm --dpi-aware DPI_SCALE 1.5 UI)
f3d_test(NAME TestDPIInvalid DATA dragon.vtu ARGS -nm --dpi-aware DPI_SCALE invalid UI NO_BASELINE)

## Color/opacity map
f3d_test(NAME TestColormap DATA IM-0001-1983.dcm ARGS --scalar-coloring --roughness=1 --colormap=0,1,0,0,1,0,1,0)
f3d_test(NAME TestOpacityMap DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-map=0.0,0.0,0.5,1.0,1.0,0.0 THRESHOLD 0.05) # Small rendering differences when using VTK 9.3.0
f3d_test(NAME TestOpacityMapFile DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/gaussian_opacity_map.png)
f3d_test(NAME TestOpacityMapFileNonExistent DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/nonexistent_opacity_map.png)
f3d_test(NAME TestOpacityMapFileInvalid DATA waveletArrays.vti ARGS -vb --coloring-array=Result --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/invalid.png)
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

## Axes grid
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11209
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250513)
  f3d_test(NAME TestAxesGridEnable DATA suzanne.ply ARGS --axes-grid THRESHOLD 0.08) # Threshold required for MacOS due to line rendering differences
endif()

## Camera
f3d_test(NAME TestCameraDirection DATA suzanne.obj ARGS --camera-direction=-xy+z)
f3d_test(NAME TestCameraClipping DATA checkerboard_colorful.obj CONFIG ${F3D_SOURCE_DIR}/testing/configs/checkerboard_colorful.json RESOLUTION 800,600)
f3d_test(NAME TestCameraOrthographic ARGS --camera-orthographic DATA cow.vtp)
f3d_test(NAME TestCameraZoomFactor DATA suzanne.obj ARGS --camera-direction=-1,-1,1 --camera-zoom-factor=1.5)
f3d_test(NAME TestCameraPersp DATA Cameras.gltf ARGS --camera-index=0)
f3d_test(NAME TestCameraOrtho DATA Cameras.gltf ARGS --camera-index=1)
f3d_test(NAME TestCameraIndexConfiguration DATA Cameras.gltf ARGS --camera-index=0  --camera-azimuth-angle=15 --camera-position=0.7,0.5,3)
f3d_test(NAME TestCameraIndexInvalid DATA Cameras.gltf ARGS --camera-index=3 REGEXP "is higher than the number of available camera" NO_BASELINE)
f3d_test(NAME TestCameraIndexNegative DATA Cameras.gltf ARGS --camera-index=-1 REGEXP "Invalid camera index" NO_BASELINE)
f3d_test(NAME TestVerboseCamera DATA Cameras.gltf ARGS --camera-index=1 --verbose NO_RENDER REGEXP "0:.*1:")

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestCameraConfiguration DATA suzanne.obj ARGS --camera-position=0,0,-10 -x --camera-view-up=+X --camera-focal-point=1,0,0 --camera-view-angle=20 --camera-azimuth-angle=40 --camera-elevation-angle=-80 --camera-direction=12,34,56 --camera-zoom-factor=78 UI)
endif()

## Volume
f3d_test(NAME TestVolume DATA HeadMRVolume.mhd ARGS -v --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1)
f3d_test(NAME TestVolumeInverse DATA HeadMRVolume.mhd ARGS -vi --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 THRESHOLD 0.05) # Small rendering differences due to volume rendering
f3d_test(NAME TestVolumeMag DATA vase_4comp.vti ARGS -vb)
f3d_test(NAME TestVolumeComp DATA vase_4comp.vti ARGS -vb --coloring-component=3 LONG_TIMEOUT)
f3d_test(NAME TestVolumeDirect DATA vase_4comp.vti ARGS -vb --coloring-component=-2)
f3d_test(NAME TestVolumeCells DATA waveletArrays.vti ARGS -vb --coloring-by-cells)
f3d_test(NAME TestVolumeColoringArray DATA waveletArrays.vti ARGS -vb --coloring-array=Result LONG_TIMEOUT)

## Textures
f3d_test(NAME TestTextureNormal DATA WaterBottle.glb ARGS --texture-normal=${F3D_SOURCE_DIR}/testing/data/normal.png --normal-scale=0.1)
f3d_test(NAME TestTextureMaterial DATA WaterBottle.glb ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/red_mod.jpg --roughness=1 --metallic=1)
f3d_test(NAME TestTextureMaterialWithOptions DATA WaterBottle.glb ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/red_mod.jpg --roughness=0.5 --metallic=0.5)
f3d_test(NAME TestTextureEmissive DATA WaterBottle.glb ARGS --texture-emissive=${F3D_SOURCE_DIR}/testing/data/red.jpg --emissive-factor=0.1,0.1,0.1)
f3d_test(NAME TestTextures DATA WaterBottle.glb ARGS --texture-material=${F3D_SOURCE_DIR}/testing/data/red.jpg --roughness=1 --metallic=1 --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo.png --texture-normal=${F3D_SOURCE_DIR}/testing/data/normal.png --texture-emissive=${F3D_SOURCE_DIR}/testing/data/red.jpg --emissive-factor=0.1,0.1,0.1)
f3d_test(NAME TestTextureMatCap DATA suzanne.ply ARGS --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png)
f3d_test(NAME TestTexturesTransform DATA world.obj ARGS --textures-transform=1,0,0,0,-1,0,0,0,1)
f3d_test(NAME TestTexturesTransformGL DATA WaterBottle.glb ARGS --textures-transform=1,0,0,0,-1,0,0,0,1 --camera-direction=-1,0,0)
f3d_test(NAME TestTextureMatCapWithEdges DATA suzanne.ply ARGS -e --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png)
f3d_test(NAME TestTextureColorWithOptions DATA WaterBottle.glb ARGS --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo_mod.png --color=1,1,0 --opacity=0.4 --blending)

if(NOT APPLE OR VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
  f3d_test(NAME TestTextureColor DATA WaterBottle.glb ARGS --texture-base-color=${F3D_SOURCE_DIR}/testing/data/albedo_mod.png --blending)
endif()

## Lights
f3d_test(NAME TestLightIntensityBrighter DATA cow.vtp ARGS --light-intensity=5.0)
f3d_test(NAME TestLightIntensityDarker DATA cow.vtp ARGS --light-intensity=0.2)
f3d_test(NAME TestLightIntensityBrighterFullScene DATA WaterBottle.glb ARGS --light-intensity=5.0)
f3d_test(NAME TestLightIntensityDarkerFullScene DATA WaterBottle.glb ARGS --light-intensity=0.2)

## Other UI
f3d_test(NAME TestUTF8 DATA "(ノಠ益ಠ )ノ.vtp")
f3d_test(NAME TestFilenameCommasSpaces DATA "tetrahedron, with commas & spaces.stl")
f3d_test(NAME TestFilename DATA suzanne.ply ARGS -n UI)
f3d_test(NAME TestHDRIFilename DATA dragon.vtu ARGS --hdri-filename -f -j --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr UI LONG_TIMEOUT)
f3d_test(NAME TestFilenameHDRIFilename DATA dragon.vtu ARGS --hdri-filename -n -f -j --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr RESOLUTION 400,400 UI LONG_TIMEOUT)
f3d_test(NAME TestHDRIFilenameDefault DATA dragon.vtu ARGS --hdri-filename -f -j UI LONG_TIMEOUT)
f3d_test(NAME TestFilenameWhiteBg DATA suzanne.ply ARGS -n --background-color=1,1,1 UI)
f3d_test(NAME TestConsoleBadgeWarning DATA suzanne.ply ARGS --position=0 UI)
f3d_test(NAME TestConsoleBadgeQuiet DATA suzanne.ply ARGS --position=0 --verbose=quiet UI)

# Require improved importer support https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11303
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240910)
  f3d_test(NAME TestConsoleBadgeError DATA invalid.vtp NO_DATA_FORCE_RENDER UI)
endif()

## Axis widget
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestAxis DATA suzanne.ply ARGS -x UI)
  f3d_test(NAME TestAxesCustomColor DATA suzanne.ply ARGS -x -g --x-color=1,0,0.643 --y-color=0,1,0.894 --z-color=0.89,1,0 UI)
endif()

## Metadata
f3d_test(NAME TestMetaData DATA pdiag.vtu ARGS -m UI)
f3d_test(NAME TestMetaDataImporter DATA BoxAnimated.gltf ARGS -m UI)
f3d_test(NAME TestMultiblockMetaData DATA mb.vtm ARGS -m UI)

## Special files handling
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
f3d_test(NAME TestNonExistentFile DATA nonExistentFile.vtp ARGS --filename WILL_FAIL)
f3d_test(NAME TestUnsupportedFile DATA unsupportedFile.dummy ARGS --filename WILL_FAIL)
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

# Require improved importer support https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11303
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240910)
  f3d_test(NAME TestInvalidFileFileNameEmpty DATA invalid.vtp ARGS --filename NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileInvalid DATA cow.vtp invalid.vtp dragon.vtu ARGS --multi-file-mode=all NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileUnsupportedFilesFileName DATA unsupportedFile.dummy cow.vtp ARGS --multi-file-mode=all --filename NO_DATA_FORCE_RENDER UI)
  f3d_test(NAME TestMultiFileCameraIndex DATA Cameras.gltf CameraAnimated.glb ARGS --multi-file-mode=all --camera-index=2 --opacity=0.5 --blending)
endif()

## Font
f3d_test(NAME TestFont DATA suzanne.ply ARGS -n --font-file=${F3D_SOURCE_DIR}/testing/data/Crosterian.ttf UI)
f3d_test(NAME TestFontScale2 DATA suzanne.ply ARGS -n --font-scale=2 UI)
f3d_test(NAME TestFontScale3 DATA suzanne.ply ARGS -n --font-scale=3 UI)

## Special CLI syntax
f3d_test(NAME TestDefines DATA dragon.vtu ARGS -Dscene.up_direction=+Z --define=model.color.rgb=red)
f3d_test(NAME TestDefinesInvalid DATA dragon.vtu ARGS -Dscene.up_direction+Z REGEXP "Could not parse a define" NO_BASELINE)
f3d_test(NAME TestDefinesInexistent DATA dragon.vtu ARGS -Dscene.up_director=+Z REGEXP "option from CLI options does not exists" NO_BASELINE)
f3d_test(NAME TestAlternativeOptionSyntax DATA WaterBottle.glb ARGS --max-size 0.2 REGEXP "file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestCustomOptionsNone DATA red_translucent_monkey.gltf ARGS --blending=none --anti-aliasing=none --point-sprites=none)

## Config
f3d_test(NAME TestConfigReset DATA suzanne.stl ARGS -Rrender.grid.enable --reset=ui.axis CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json)
f3d_test(NAME TestConfigResetInexistent DATA suzanne.stl ARGS -Rrender.glid.enable REGEXP "option from CLI options does not exists" NO_BASELINE)
f3d_test(NAME TestConfigOrder DATA suzanne.ply ARGS CONFIG ${F3D_SOURCE_DIR}/testing/configs/config_order.json) # `.+` > `.*` alphabetically but overridden by the order
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestConfigImperative DATA dragon.vtu ARGS --axis=false CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json UI)
endif()

## Animation
f3d_test(NAME TestAnimationIndicesSingle DATA soldier_animations.mdl ARGS --animation-indices=7 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestAnimationIndicesMulti DATA InterpolationTest.glb ARGS --animation-indices=7,6 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestAnimationIndexDeprecated DATA InterpolationTest.glb ARGS --animation-index=7 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestMultiFileAnimationIndices DATA InterpolationTest.glb BoxAnimated.gltf ARGS --animation-indices=9 --animation-time=0.85 --animation-progress --multi-file-mode=all)
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12688
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251006)
  f3d_test(NAME TestMultiFileAnimationNoneMulti DATA bot2.wrl InterpolationTest.glb ARGS --animation-indices=6 --animation-time=0.85 --multi-file-mode=all)
endif()
f3d_test(NAME TestMultiFileAnimationAnySingle DATA soldier_animations.mdl InterpolationTest.glb ARGS --animation-indices=13 --animation-time=0.85 --multi-file-mode=all --opacity=0.5)
f3d_test(NAME TestMultiFileAnimationNoAnimationSupport DATA f3d.glb world.obj ARGS --multi-file-mode=all --animation-time=2 --animation-progress)
f3d_test(NAME TestAnimationAutoplay DATA InterpolationTest.glb ARGS --animation-autoplay)
f3d_test(NAME TestAnimationAllAnimations DATA InterpolationTest.glb ARGS --animation-indices=-1 --animation-time=1 --animation-progress)
f3d_test(NAME TestAnimationNoAnimations DATA InterpolationTest.glb ARGS --animation-indices= --verbose NO_BASELINE REGEXP "Current animation is: No animation")
f3d_test(NAME TestVerboseAnimation DATA InterpolationTest.glb ARGS --verbose NO_BASELINE REGEXP "7: CubicSpline Translation")
f3d_test(NAME TestVerboseAnimationWrongAnimationTimeHigh DATA BoxAnimated.gltf ARGS --animation-time=10 --verbose REGEXP "Animation time 10 is outside of range \\[0, 3\\.70833\\], using 3\\.70833" NO_BASELINE)
f3d_test(NAME TestVerboseAnimationWrongAnimationTimeLow DATA BoxAnimated.gltf ARGS --animation-time=-5 --verbose REGEXP "Animation time -5 is outside of range \\[0, 3\\.70833\\], using 0" NO_BASELINE)
f3d_test(NAME TestVerboseAnimationTimeRange DATA InterpolationTest.glb ARGS --verbose REGEXP "0, 1.66667" NO_BASELINE)
f3d_test(NAME TestCommandScriptVerboseMultiAnimationTimeRange SCRIPT DATA InterpolationTest.glb ARGS --verbose REGEXP "0, 1.70833" NO_BASELINE)# cycle_animation x3

## MaxSize
f3d_test(NAME TestMaxSizeBelow DATA suzanne.stl ARGS --max-size=1)
f3d_test(NAME TestMaxSizeAbove DATA WaterBottle.glb ARGS --max-size=0.2 REGEXP "file is bigger than max size" NO_BASELINE)
f3d_test(NAME TestMaxSizeAboveMultiFile DATA suzanne.obj WaterBottle.glb ARGS --multi-file-mode=all --max-size=0.6 --blending --opacity=0.5)

## NoRender
f3d_test(NAME TestNoRender DATA dragon.vtu NO_RENDER)
f3d_test(NAME TestNoRenderWithOptions DATA dragon.vtu ARGS --hdri-ambient --axis NO_RENDER) # These options causes issues if not handled correctly

## Up
f3d_test(NAME TestInvalidUpDirection DATA suzanne.ply ARGS -g --up=W REGEXP "Could not set 'up'" NO_BASELINE)
f3d_test(NAME TestUpDirectionNoSign DATA suzanne.ply ARGS --up=X)
f3d_test(NAME TestCollinearVectors DATA dragon.vtu ARGS --up=0,1,0 --camera-direction=0,2,0)

## OutputStream
f3d_test(NAME TestOutputStream DATA suzanne.ply ARGS --verbose=quiet --output=- REGEXP ".PNG" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestOutputStreamInfo DATA suzanne.ply ARGS --verbose=info --output=- REGEXP "redirected to stderr" NO_BASELINE NO_OUTPUT)

## AntiAliasing
f3d_test(NAME TestInvalidAntiAliasingMode DATA suzanne.ply ARGS --anti-aliasing=foo REGEXP "foo is an invalid antialiasing mode" NO_BASELINE)
f3d_test(NAME TestAntiAliasingModeDeprecated DATA suzanne.ply ARGS -a --anti-aliasing-mode=ssaa REGEXP "--anti-aliasing-mode is deprecated" NO_BASELINE)
f3d_test(NAME TestAntiAliasingDeprecated DATA suzanne.ply ARGS --anti-aliasing=true REGEXP "please specify the type of anti-aliasing" NO_BASELINE)

## Blending
f3d_test(NAME TestInvalidBlendingMode DATA suzanne.ply ARGS --blending=foo REGEXP "foo is an invalid blending mode" NO_BASELINE)
f3d_test(NAME TestTranslucencySupportDeprecated DATA suzanne.ply ARGS --translucency-support REGEXP "--translucency-support is deprecated" NO_BASELINE)

## InteractionStyle
f3d_test(NAME TestInteractionTrackballDeprecated DATA suzanne.ply ARGS --interaction-trackball REGEXP "is deprecated" NO_BASELINE)
f3d_test(NAME TestInteractionTrackballInvalid DATA suzanne.ply ARGS --interaction-trackball=foo REGEXP "Cannot parse --interaction-trackball value" NO_BASELINE)

## PointSprites
f3d_test(NAME TestInvalidPointSprites DATA suzanne.ply ARGS --point-sprites=foo REGEXP "foo is an invalid point sprites type" NO_BASELINE)
f3d_test(NAME TestPointSpritesTypeDeprecated DATA pointsCloud.vtp ARGS --point-sprites-type=sphere REGEXP "--point-sprites-type is deprecated" NO_BASELINE)
f3d_test(NAME TestPointSpritesDeprecated DATA pointsCloud.vtp ARGS --point-sprites=true REGEXP "please specify the type of point sprites" NO_BASELINE)
f3d_test(NAME TestAnimationUserMatrixPointSprites DATA BoxAnimated.gltf ARGS --point-sprites --point-sprites-size=50 --animation-time=2)
f3d_test(NAME TestAnimationInputChangePointSprites DATA v_rock2.mdl ARGS --point-sprites --point-sprites-size=50 --animation-time=0.01 --animation-indices=1)

## Backdrop
f3d_test(NAME TestBackdropOpacityOpaque DATA suzanne.ply ARGS -n --backdrop-opacity=1.0 UI)
f3d_test(NAME TestBackdropOpacityTransparent DATA suzanne.ply ARGS -n --backdrop-opacity=0.0 UI)
f3d_test(NAME TestBackdropOpacityMedium DATA suzanne.ply ARGS -n --backdrop-opacity=0.5 UI)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestDefaultConfigFileHDRIFilename DATA dragon.vtu CONFIG config_build ARGS -j --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr UI LONG_TIMEOUT)
endif()

## Skinning
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

## Armature
f3d_test(NAME TestGLTFRigArmatureNoArmature DATA RiggedFigure.glb ARGS --animation-time=1 --opacity=0.5 -p)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20241219)
  f3d_test(NAME TestGLTFRigArmature DATA RiggedFigure.glb ARGS --animation-time=1 --armature)
  f3d_test(NAME TestGLTFRigArmatureWithOpacity DATA RiggedFigure.glb ARGS --animation-time=1 --armature --opacity=0.5 -p)
  f3d_test(NAME TestGLTFRigArmatureSphereTube DATA RiggedFigure.glb ARGS --animation-time=1 --armature --point-size=20 --line-width=5)
endif()

## HDRI
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

if(F3D_MODULE_RAYTRACING)
  # https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10753
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231213)
    # XXX: These tests are impacted by https://github.com/f3d-app/f3d/issues/933
    f3d_test(NAME TestHDRIRaytracing DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4)
    f3d_test(NAME TestHDRIRaytracingSkyboxOnly DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-skybox -rd --raytracing-samples=4 LONG_TIMEOUT)
    f3d_test(NAME TestHDRIRaytracingAmbientOnly DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-ambient -rd --raytracing-samples=4 LONG_TIMEOUT)
    f3d_test(NAME TestHDRIRaytracingAmbientOnlyNoBackground DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-ambient -rd --raytracing-samples=4 --no-background LONG_TIMEOUT)
    f3d_test(NAME TestHDRIRaytracingNone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr -rd --raytracing-samples=4 LONG_TIMEOUT)
  endif()
endif()

# SSAA with HDR framebuffer support in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12026
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250329)
  f3d_test(NAME TestHDRIToneMappingSSAA DATA suzanne.ply HDRI palermo_park_1k.hdr TONE_MAPPING ARGS -t --anti-aliasing=ssaa)
endif()

## Raytracing
if(F3D_MODULE_RAYTRACING)
  # Ideally we could test these with ospray 2.7 and VTK 9.3
  # https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10753
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231213)
    f3d_test(NAME TestRaytracingGLTF DATA WaterBottle.glb ARGS -rd --raytracing-samples=4)
    f3d_test(NAME TestRaytracingBackground DATA suzanne.ply ARGS -rd --raytracing-samples=4 --background-color=1,0,0 THRESHOLD 0.05) # Threshold needed because of difference in rendering in VTK 9.3
    f3d_test(NAME TestRaytracingPointCloud DATA pointsCloud.vtp ARGS -rd --raytracing-samples=4 --point-size=20 THRESHOLD 0.1) # Threshold needed because of difference in rendering in VTK 9.3
    f3d_test(NAME TestRaytracingDenoise DATA suzanne.ply ARGS -rd --raytracing-samples=4)
    f3d_test(NAME TestRaytracingNoDenoise DATA suzanne.stl ARGS -r --raytracing-samples=20)
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
  f3d_test(NAME TestCommandScriptRaytracing SCRIPT DATA suzanne.ply NO_BASELINE REGEXP "Raytracing options can't be used if F3D has not been built with raytracing")
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

## Final shader
f3d_test(NAME TestFinalShaderNegative DATA cow.vtp ARGS --final-shader "vec4 pixel(vec2 uv){return vec4(vec3(1.0) - texture(source, uv).rgb, 1.0)\\\\\\\;}")
f3d_test(NAME TestFinalShaderNegativeFileName DATA cow.vtp ARGS --filename --final-shader "vec4 pixel(vec2 uv){return vec4(vec3(1.0) - texture(source, uv).rgb, 1.0)\\\\\\\;}" UI)
f3d_test(NAME TestFinalShaderUndefined DATA cow.vtp ARGS --final-shader "undefined" REGEXP "Final shader must define a function" NO_BASELINE)
f3d_test(NAME TestFinalShaderCompilationFailure DATA cow.vtp ARGS --final-shader "vec4 pixel(vec2 uv){}" --verbose REGEXP " build the shader program" NO_BASELINE)

## Command Script
f3d_test(NAME TestCommandScriptBasic SCRIPT DATA dragon.vtu) # roll_camera 90;toggle ui.scalar_bar;print_scene_info;increase_light_intensity
f3d_test(NAME TestCommandScriptElevation SCRIPT DATA dragon.vtu) # elevation_camera 90;toggle ui.scalar_bar;print_scene_info;increase_light_intensity
f3d_test(NAME TestCommandScriptAzimuth SCRIPT DATA dragon.vtu) # azimuth_camera 90;toggle ui.scalar_bar;print_scene_info;increase_light_intensity
f3d_test(NAME TestCommandScriptInvalid SCRIPT DATA dragon.vtu REGEXP "Command: \"INVALID_COMMAND_1\" is not recognized, ignoring" NO_BASELINE) # INVALID_COMMAND_1
f3d_test(NAME TestCommandScriptMissingFile SCRIPT DATA dragon.vtu REGEXP "Unable to open command script file" NO_BASELINE)
f3d_test(NAME TestCommandScriptPrintScene SCRIPT DATA dragon.vtu REGEXP "Camera position: 2.23745, 3.83305, 507.598" NO_BASELINE) # print_scene_info
f3d_test(NAME TestCommandScriptPrintConfig SCRIPT DATA dragon.vtu REGEXP "Found available config path" NO_BASELINE) # print_config_info
f3d_test(NAME TestCommandScriptPrintColoring SCRIPT DATA dragon.vtu REGEXP "Not coloring" NO_BASELINE) # print_coloring_info
f3d_test(NAME TestCommandScriptPrintMesh SCRIPT DATA dragon.vtu REGEXP "Number of points: 13268" NO_BASELINE) # print_mesh_info
f3d_test(NAME TestCommandScriptPrintOptions SCRIPT DATA dragon.vtu REGEXP "interactor.invert_zoom: false" NO_BASELINE) # print_options_info
f3d_test(NAME TestCommandScriptAlias SCRIPT DATA dragon.vtu --reference=${F3D_SOURCE_DIR}/testing/baselines/TestCommandScriptAlias.png) # alias myrotate roll_camera 90;myrotate
f3d_test(NAME TestCommandScriptReset SCRIPT DATA dragon.vtu suzanne.stl ARGS --edges) # reset render.show_edges; load_next_file_group;
f3d_test(NAME TestCommandScriptParseOptionalBoolExtraArg SCRIPT DATA dragon.vtu REGEXP "Command: load_previous_file_group takes at most 1 argument, got 2 arguments instead." NO_BASELINE) # load_previous_file_group true extra
f3d_test(NAME TestCommandScriptRemoveCurrentFileGroup SCRIPT DATA cow.vtp dragon.vtu) # remove_current_file_group
f3d_test(NAME TestCommandScriptRemoveFileGroups SCRIPT DATA dragon.vtu NO_DATA_FORCE_RENDER UI) # remove_file_groups
f3d_test(NAME TestCommandScriptInvalidReaderOptions SCRIPT DATA dragon.vtu REGEXP "point to an inexistent option, ignoring" NO_BASELINE) # set_reader_option invalid value
f3d_test(NAME TestCommandScriptHelp SCRIPT DATA dragon.vtu REGEX "set a libf3d option" NO_BASELINE) # help set
f3d_test(NAME TestCommandScriptHelpInvalid SCRIPT DATA dragon.vtu REGEX "is not a recognized command" NO_BASELINE) # help invalid
f3d_test(NAME TestCommandScriptJumpToPreviousFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToNextFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToFirstFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToLastFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToMiddleFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-time=0.5 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToPreviousKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToNextKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToFirstKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToAbsoluteKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
if (F3D_PLUGIN_BUILD_ASSIMP AND F3D_ASSIMP_VERSION VERSION_GREATER_EQUAL "6.0.1")
  # TODO: Update this test to NOT use assimp once generic importer supports timesteps properly (issue: https://github.com/f3d-app/f3d/issues/2733)
  f3d_test(NAME TestCommandScriptJumpToAbsoluteKeyFrameMultipleAnimations SCRIPT DATA punch.fbx soldier_animations.mdl ARGS --load-plugins=assimp --multi-file-mode=all --animation-indices=0,2 --animation-progress)
endif()
f3d_test(NAME TestCommandScriptJumpToClosestKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-time=0.14 --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToStartKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-time=0.4 --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToPositiveOutsideKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToNegativeOutsideKeyFrame SCRIPT DATA soldier_animations.mdl ARGS --animation-indices=2 --animation-progress)
f3d_test(NAME TestCommandScriptJumpToKeyFrameNoAnimation SCRIPT DATA cow.vtp)

## Tests to increase coverage
# Output option test
f3d_test(NAME TestOutput DATA cow.vtp NO_BASELINE)
f3d_test(NAME TestOutputOutput DATA cow.vtp ARGS --reference=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutput.png DEPENDS TestOutput NO_BASELINE)
f3d_test(NAME TestUnsupportedInputOutput DATA unsupportedFile.dummy REGEXP "No files loaded, no rendering performed" NO_BASELINE)
f3d_test(NAME TestOutputNoBackground DATA cow.vtp ARGS --no-background NO_BASELINE)

# Multi-frame output tests
f3d_test(NAME TestOutputFrameCount DATA BoxAnimated.gltf ARGS --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutputFrameCount_{frame:4}.png --frame-rate=0.25 REGEXP "Saved 2 animation frame" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestOutputFrameCountFrame0 DATA BoxAnimated.gltf ARGS --reference=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutputFrameCount_0000.png --animation-time=0 DEPENDS TestOutputFrameCount NO_BASELINE)
f3d_test(NAME TestOutputFrameCountFrame1 DATA BoxAnimated.gltf ARGS --reference=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutputFrameCount_0001.png --animation-time=3.70833 DEPENDS TestOutputFrameCount NO_BASELINE)
f3d_test(NAME TestOutputFrameCountNoAnimation DATA cow.vtp ARGS --output=${CMAKE_BINARY_DIR}/Testing/Temporary/static_{frame:4}.png REGEXP "No animation available" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestOutputFrameCountInvalidFormat DATA BoxAnimated.gltf ARGS --output=${CMAKE_BINARY_DIR}/Testing/Temporary/invalid_{frame:abc}.png --frame-rate=0.25 REGEXP "ignoring invalid frame format" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestOutputFrameCountStartTime DATA BoxAnimated.gltf ARGS --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestOutputFrameCountStartTime_{frame:4}.png --frame-rate=0.3 --animation-time=2.0 REGEXP "Saving 2 animation frame" NO_BASELINE NO_OUTPUT)
f3d_test(NAME TestCommandScriptScreenshotFrame SCRIPT DATA cow.vtp ARGS --screenshot-filename=${CMAKE_BINARY_DIR}/Testing/Temporary/screenshot_{frame}.png REGEXP "{frame} variable can only be used when outputting animation frames" NO_BASELINE)

# Basic record and play test
f3d_test(NAME TestInteractionRecord DATA cow.vtp ARGS --interaction-test-record=${CMAKE_BINARY_DIR}/Testing/Temporary/TestInteractionRecord.log NO_BASELINE)
f3d_test(NAME TestInteractionPlay DATA cow.vtp ARGS --interaction-test-play=${CMAKE_BINARY_DIR}/Testing/Temporary/TestInteractionRecord.log DEPENDS TestInteractionRecord NO_BASELINE)

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
f3d_test(NAME TestAnimationIndicesWarningRange DATA InterpolationTest.glb ARGS --animation-indices=48 NO_BASELINE REGEXP "Specified animation index: 48 is not in range")
f3d_test(NAME TestAnimationIndicesWarningNone DATA cow.vtp ARGS --animation-indices=1 REGEXP "Animation indices have been specified but there are no animation available" NO_BASELINE)
f3d_test(NAME TestAnimationIndicesWarningAllAnimations DATA InterpolationTest.glb ARGS --animation-indices=-1,2,3 REGEXP "Multiple animation indices have been specified include a negative one, all animations will be selected" NO_BASELINE)
f3d_test(NAME TestVerboseAnimationNoAnimationTime DATA cow.vtp ARGS --animation-time=2 --verbose REGEXP "No animation available, cannot load a specific animation time" NO_BASELINE)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250507)
  f3d_test(NAME TestAnimationIndicesWarningSingle DATA soldier_animations.mdl ARGS --animation-indices=1,2 REGEXP "Multiple animation indices have been specified but currently loaded files may not support enabling multiple animations" NO_BASELINE)
  if(F3D_PLUGIN_BUILD_HDF)
    f3d_test(NAME TestAnimationIndicesWarningUniqueNonZero DATA blob.vtkhdf ARGS --animation-indices=1 --load-plugins=hdf REGEXP "Non-zero or multiple animation indices have been specified but currently loaded file does not support it." NO_BASELINE)
    f3d_test(NAME TestAnimationIndicesWarningUniqueMulti DATA blob.vtkhdf ARGS --animation-indices=0,2 --load-plugins=hdf REGEXP "Non-zero or multiple animation indices have been specified but currently loaded file does not support it." NO_BASELINE)
  endif()
endif()

# Test Grid verbose output
f3d_test(NAME TestVerboseGrid DATA suzanne.ply ARGS -g --verbose REGEXP "Grid origin set to" NO_BASELINE)

# Test bounding box no render output
f3d_test(NAME TestNoRenderBBox DATA suzanne.ply NO_RENDER REGEXP "Scene bounding box: -1.32819 ≤ x ≤ 1.32819, -0.971822 ≤ y ≤ 0.939236, -0.778266 ≤ z ≤ 0.822441")

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
f3d_test(NAME TestVerboseNonExistentFile DATA nonExistentFile.vtp REGEXP ".*nonExistentFile.vtp does not exist" NO_RENDER)

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
f3d_test(NAME TestConfigFileInexistentKey DATA cow.vtp CONFIG ${F3D_SOURCE_DIR}/testing/configs/inexistent_key.json REGEXP "does not exists , did you mean 'scene.animation.indices" NO_BASELINE)

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
# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestMatchFirstConfigGlobOptionBlock DATA dragon.vtu CONFIG ${CMAKE_BINARY_DIR}/glob.json UI)
endif()
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
f3d_test(NAME TestHelpVersionPresent ARGS --help REGEXP "--version")

# Test version display
f3d_test(NAME TestVersion ARGS --version REGEXP "Version:")

# Test list-readers display
f3d_test(NAME TestReadersList ARGS --list-readers REGEXP_FAIL "No registered reader found")

# Test invalid component string coverage
f3d_test(NAME TestInteractionInvalidComponent INTERACTION UI DATA cow.vtp ARGS --coloring-component=1 NO_BASELINE) #H

# Test opening invalid file then switching to another file
f3d_test(NAME TestInteractionInvalidFile INTERACTION DATA invalid.vtp cow.vtp ARGS --loading-progress) #Right

# Test bindings-list display
f3d_test(NAME TestBindingsList ARGS --list-bindings REGEXP "Any.5        Orthographic Projection")

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

## Filesystem error code path
if(NOT WIN32)
  STRING(RANDOM LENGTH 257 ALPHABET "x" _f3d_test_invalid_folder)
  f3d_test(NAME TestColorMapTooLong DATA dragon.vtu ARGS --colormap-file=${_f3d_test_invalid_folder}/file.ext --scalar-coloring REGEXP "File name too long" NO_BASELINE)
  f3d_test(NAME TestScreenshotTooLong DATA suzanne.ply ARGS --screenshot-filename=${_f3d_test_invalid_folder}/file.ext --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log REGEXP "File name too long" NO_BASELINE)
  f3d_test(NAME TestInputTooLong ARGS --input=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE)
  f3d_test(NAME TestReferenceTooLong DATA suzanne.ply ARGS --output=file.png --reference=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputTooLong DATA suzanne.ply ARGS --output=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputFrameCountTooLong DATA BoxAnimated.gltf ARGS --output=${_f3d_test_invalid_folder}/frame_{frame}.png --frame-rate=0.25 REGEXP "Could not write output" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputFrameCountNoAnimationTooLong DATA cow.vtp ARGS --output=${_f3d_test_invalid_folder}/frame_{frame}.png REGEXP "Could not write output" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputWithReferenceTooLong DATA suzanne.ply ARGS --reference=file.png --output=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestOutputWithExistingReferenceTooLong DATA suzanne.ply ARGS --reference=${F3D_SOURCE_DIR}/testing/data/world.png --output=${_f3d_test_invalid_folder}/file.ext REGEXP "File name too long" NO_BASELINE NO_OUTPUT)
  f3d_test(NAME TestConfigTooLong CONFIG ${_f3d_test_invalid_folder}/invalid.json REGEXP "File name too long" NO_RENDER NO_BASELINE)
endif()
