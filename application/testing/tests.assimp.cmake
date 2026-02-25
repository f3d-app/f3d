## Tests that needs assimp plugin
## This file is only added if assimp is enabled
f3d_test(NAME TestOFF DATA teapot.off ARGS --up=+Z PLUGIN assimp)
f3d_test(NAME Test3MF DATA cube_gears.3mf PLUGIN assimp)
f3d_test(NAME TestFBX DATA phong_cube.fbx PLUGIN assimp)
f3d_test(NAME TestFBX16bits DATA 16bit.fbx PLUGIN assimp)
f3d_test(NAME TestVerboseCameraAssimp DATA duck.dae ARGS --verbose PLUGIN assimp NO_BASELINE REGEXP "camera1")
f3d_test(NAME TestDXF DATA PinkEggFromLW.dxf ARGS --background-color=1,1,1 -p PLUGIN assimp)

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_test(NAME TestAssimpInvalid DATA invalid_truncated.fbx PLUGIN assimp REGEXP "Some of these files could not be loaded" NO_BASELINE)
  f3d_test(NAME TestAssimpMetaDataImporter DATA duck.fbx PLUGIN assimp ARGS -m UI)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251016)
  f3d_test(NAME TestPipedOFF DATA teapot.off ARGS --up=+Z PLUGIN assimp PIPED OFF)
  f3d_test(NAME TestPiped3MF DATA cube_gears.3mf PLUGIN assimp PIPED 3MF)
  f3d_test(NAME TestPipedFBX DATA phong_cube.fbx PLUGIN assimp PIPED FBX)
  f3d_test(NAME TestPipedDAE DATA duck.dae PLUGIN assimp PIPED COLLADA) # Texture not loaded
  f3d_test(NAME TestPipedX DATA anim_test.x PLUGIN assimp PIPED DirectX)
  f3d_test(NAME TestPipedDXF DATA PinkEggFromLW.dxf ARGS --background-color=1,1,1 -p PLUGIN assimp PIPED DXF)
endif()

f3d_test(NAME TestVerboseAssimp DATA duck.fbx ARGS --verbose PLUGIN assimp NO_BASELINE REGEXP "LOD3sp")
f3d_test(NAME TestVerboseAssimpAnimationIndicesError DATA animatedLights.fbx PLUGIN assimp ARGS --animation-indices=48 NO_BASELINE REGEXP "Specified animation index: 48 is not in range")

f3d_test(NAME TestAssimpAnimationNegativeIndex DATA animatedLights.fbx PLUGIN assimp ARGS --animation-indices=-113 --animation-time=2 --animation-progress)
f3d_test(NAME TestTGATextureFBX DATA duck.fbx PLUGIN assimp)
f3d_test(NAME TestDAE DATA duck.dae PLUGIN assimp)
f3d_test(NAME TestX DATA anim_test.x PLUGIN assimp)

# This test baseline is incorrect because of https://github.com/f3d-app/f3d/issues/603
# It will need to be changed when fixed
f3d_test(NAME TestFBXNormalMapping DATA normalMapping.fbx PLUGIN assimp)

# Tests using embedded textures
f3d_test(NAME TestEmbeddedTextureFBX DATA texturedCube.fbx PLUGIN assimp)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12688
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251006)
  f3d_test(NAME TestFBXAnimation DATA animatedWorld.fbx PLUGIN assimp ARGS --animation-time=2 --animation-progress)
  f3d_test(NAME TestFBXAnimationLights DATA animatedLights.fbx PLUGIN assimp ARGS --animation-time=1.8 --animation-progress)
  f3d_test(NAME TestFBXAnimationCamera DATA animatedCamera.fbx PLUGIN assimp ARGS --camera-index=0 --animation-indices=0 --animation-time=3 --animation-progress)
  f3d_test(NAME TestDAEAnimationLights DATA animatedLights.dae PLUGIN assimp ARGS --animation-time=1.8 --animation-progress)
endif()

if("${F3D_ASSIMP_VERSION}" VERSION_GREATER_EQUAL "5.4.3")
  f3d_test(NAME TestFBXBone DATA animation_with_skeleton.fbx PLUGIN assimp ARGS --camera-position=1.90735e-06,0,11007.8 --camera-focal-point=1.90735e-06,0,-8.9407e-08)
  f3d_test(NAME TestFBXBoneAnimation DATA animation_with_skeleton.fbx PLUGIN assimp ARGS --camera-position=1.90735e-06,0,11007.8 --camera-focal-point=1.90735e-06,0,-8.9407e-08 --animation-time=0.5 --animation-progress)
  f3d_test(NAME TestInteractionAnimationFBXBone DATA animation_with_skeleton.fbx PLUGIN assimp ARGS --camera-position=0,0,14000 --camera-focal-point=0,0,0 INTERACTION) #Space;Wait;Space;
endif()

# The visible boxes are not located at the same position in Assimp 6.0 for some reason
if("${F3D_ASSIMP_VERSION}" VERSION_GREATER_EQUAL "6.0.1")
  f3d_test(NAME TestFBXSkinningAnimation DATA punch.fbx PLUGIN assimp ARGS --animation-time=1 --animation-progress)
endif()

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/assimp/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileAssimpFBX DATA phong_cube.fbx CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;assimp")
    f3d_test(NAME TestDefaultConfigFileAssimpDXF DATA PinkEggFromLW.dxf ARGS -p CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;assimp")
    f3d_test(NAME TestDefaultConfigFileAssimpOFF DATA teapot.off CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;assimp")
    f3d_test(NAME TestDefaultConfigFileAssimpDAE DATA duck.dae CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;assimp")
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/assimp/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileAssimpFBX DATA phong_cube.fbx CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;assimp")
  f3d_test(NAME TestThumbnailConfigFileAssimpDXF DATA PinkEggFromLW.dxf ARGS -p CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;assimp")
  f3d_test(NAME TestThumbnailConfigFileAssimpOFF DATA teapot.off CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;assimp")
  f3d_test(NAME TestThumbnailConfigFileAssimpDAE DATA duck.dae CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;assimp")
endif()
