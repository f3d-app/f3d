## Tests that needs assimp plugin
## This file is only added if assimp is enabled
f3d_test(NAME TestOFF DATA teapot.off ARGS --up=+Z --load-plugins=assimp)
f3d_test(NAME Test3MF DATA cube_gears.3mf ARGS --load-plugins=assimp)
f3d_test(NAME TestFBX DATA phong_cube.fbx ARGS --load-plugins=assimp)
f3d_test(NAME TestFBX16bits DATA 16bit.fbx ARGS --load-plugins=assimp)
f3d_test(NAME TestVerboseCameraAssimp DATA duck.dae ARGS --verbose --load-plugins=assimp NO_BASELINE REGEXP "camera1")

if(NOT APPLE OR VTK_VERSION VERSION_GREATER_EQUAL 9.3.0)
  f3d_test(NAME TestDXF DATA PinkEggFromLW.dxf ARGS --background-color=1,1,1 -p --load-plugins=assimp)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240707)
  f3d_test(NAME TestAssimpValidation DATA Wolf.fbx ARGS --load-plugins=assimp REGEXP "Some of these files could not be loaded" NO_BASELINE)
  f3d_test(NAME TestAssimpMetaDataImporter DATA duck.fbx ARGS --load-plugins=assimp -m UI)
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251016)
  f3d_test(NAME TestPipedOFF DATA teapot.off ARGS --up=+Z --load-plugins=assimp --force-reader=OFF PIPED)
  f3d_test(NAME TestPiped3MF DATA cube_gears.3mf ARGS --load-plugins=assimp --force-reader=3MF PIPED)
  f3d_test(NAME TestPipedFBX DATA phong_cube.fbx ARGS --load-plugins=assimp --force-reader=FBX PIPED)
  f3d_test(NAME TestPipedDAE DATA duck.dae ARGS --load-plugins=assimp --force-reader=COLLADA PIPED) # Texture not loaded
  f3d_test(NAME TestPipedX DATA anim_test.x ARGS --load-plugins=assimp --force-reader=DirectX PIPED)
  if (NOT APPLE)
    f3d_test(NAME TestPipedDXF DATA PinkEggFromLW.dxf ARGS --background-color=1,1,1 -p --load-plugins=assimp --force-reader=DXF PIPED)
  endif()
endif()

f3d_test(NAME TestVerboseAssimp DATA duck.fbx ARGS --verbose --load-plugins=assimp NO_BASELINE REGEXP "LOD3sp")
f3d_test(NAME TestVerboseAssimpAnimationIndicesError DATA animatedLights.fbx ARGS --load-plugins=assimp --animation-indices=48 NO_BASELINE REGEXP "Specified animation index: 48 is not in range")

f3d_test(NAME TestAssimpAnimationNegativeIndex DATA animatedLights.fbx ARGS --load-plugins=assimp --animation-indices=-113 --animation-time=2 --animation-progress)
f3d_test(NAME TestTGATextureFBX DATA duck.fbx ARGS --load-plugins=assimp)
f3d_test(NAME TestDAE DATA duck.dae ARGS --load-plugins=assimp)
f3d_test(NAME TestX DATA anim_test.x ARGS --load-plugins=assimp)

# This test baseline is incorrect because of https://github.com/f3d-app/f3d/issues/603
# It will need to be changed when fixed
f3d_test(NAME TestFBXNormalMapping DATA normalMapping.fbx ARGS --load-plugins=assimp)

# Tests using embedded textures
f3d_test(NAME TestEmbeddedTextureFBX DATA texturedCube.fbx ARGS --load-plugins=assimp)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12688
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251006)
  f3d_test(NAME TestFBXAnimation DATA animatedWorld.fbx ARGS --load-plugins=assimp --animation-time=2 --animation-progress)
  f3d_test(NAME TestFBXAnimationLights DATA animatedLights.fbx ARGS --load-plugins=assimp --animation-time=1.8 --animation-progress)
  f3d_test(NAME TestFBXAnimationCamera DATA animatedCamera.fbx ARGS --load-plugins=assimp --camera-index=0 --animation-indices=0 --animation-time=3 --animation-progress)
  f3d_test(NAME TestDAEAnimationLights DATA animatedLights.dae ARGS --load-plugins=assimp --animation-time=1.8 --animation-progress)
endif()

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
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestDefaultConfigFileAssimpFBX DATA phong_cube.fbx CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAssimpDXF DATA PinkEggFromLW.dxf ARGS -p CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAssimpOFF DATA teapot.off CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
    f3d_test(NAME TestDefaultConfigFileAssimpDAE DATA duck.dae CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI)
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/assimp/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_test(NAME TestThumbnailConfigFileAssimpFBX DATA phong_cube.fbx CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileAssimpDXF DATA PinkEggFromLW.dxf ARGS -p CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileAssimpOFF DATA teapot.off CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
  f3d_test(NAME TestThumbnailConfigFileAssimpDAE DATA duck.dae CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING)
endif()
