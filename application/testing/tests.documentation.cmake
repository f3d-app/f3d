# Tests the documentation illustrations in doc/user

## Download required files
### 3D model with nice materials: DamagedHelmet.glb
file(DOWNLOAD
    "https://f3d.app/data/DamagedHelmet.glb"
    ${CMAKE_CURRENT_BINARY_DIR}/DamagedHelmet.glb
    EXPECTED_HASH "SHA256=a1e3b04de97b11de564ce6e53b95f02954a297f0008183ac63a4f5974f6b32d8"
    SHOW_PROGRESS
)
### 3D model with armature: bristleback_dota_fan-art.glb
file(DOWNLOAD
    "https://f3d.app/data/bristleback_dota_fan-art.glb"
    ${CMAKE_CURRENT_BINARY_DIR}/bristleback_dota_fan-art.glb
    EXPECTED_HASH "SHA256=c47130fe6df38a882ef132f54dc9bd392e5267701c762a87e8aa2d40bc9cc954"
    SHOW_PROGRESS
)
### Simple 3D model: dragon.vtu
file(DOWNLOAD
    "https://f3d.app/data/dragon.vtu"
    ${CMAKE_CURRENT_BINARY_DIR}/dragon.vtu
    EXPECTED_HASH "SHA256=934a8fb81257275f8c034feec8dca771192c40e06b67a49108ff9e3fbd260ff2"
    SHOW_PROGRESS
)
### Volumetric data with scalar array: skull.vti
file(DOWNLOAD
    "https://f3d.app/data/skull.vti"
    ${CMAKE_CURRENT_BINARY_DIR}/skull.vti
    EXPECTED_HASH "SHA256=8c2cdb8cc9e6a5c6b8c0a8e7db60a451cf53df34e2365a1916ef2dcdb33f39cf"
    SHOW_PROGRESS
)
### HDRI: future_parking_2k.hdr
file(DOWNLOAD
    "https://f3d.app/data/future_parking_2k.hdr"
    ${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr
    EXPECTED_HASH "SHA256=8621abbe080b52a2abe7e9a20c8019466b0664290069bfd040a4897df490c80d"
    SHOW_PROGRESS
)
### Point cloud: pointCloud.vtp
file(DOWNLOAD
    "https://f3d.app/data/pointCloud.vtp"
    ${CMAKE_CURRENT_BINARY_DIR}/pointCloud.vtp
    EXPECTED_HASH "SHA256=31c5f3cc1ffaa17339c9d85ba5bed9072a4e8d14431dac3765fc3d599cc525d4"
    SHOW_PROGRESS
)

## Wrapper that sets many default arguments for documentation illustration tests
function(f3d_test_doc)
  cmake_parse_arguments(F3D_TEST_DOC "UI;BACKGROUND;ROTATE" "NAME;DATA;REF_IMAGE;THRESHOLD" "ARGS" ${ARGN})

  if(NOT F3D_TEST_DOC_THRESHOLD)
    set(F3D_TEST_DOC_THRESHOLD 0.04)
  endif()

  set(_f3d_test_doc_args
    --anti-aliasing=ssaa
    --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr
    ${F3D_TEST_DOC_ARGS}
  )
  if(NOT F3D_TEST_DOC_BACKGROUND)
    list(APPEND _f3d_test_doc_args --no-background)
  endif()

  if(F3D_TEST_DOC_ROTATE)
    list(APPEND _f3d_test_doc_args --camera-direction=-1,-0.5,-1)
  endif()

  cmake_path(IS_ABSOLUTE F3D_TEST_DOC_DATA _f3d_doc_data_absolute)
  set(_f3d_doc_data ${F3D_TEST_DOC_DATA})
  if (NOT _f3d_doc_data_absolute)
    set(_f3d_doc_data ${CMAKE_CURRENT_BINARY_DIR}/${F3D_TEST_DOC_DATA})
  endif()

  f3d_test(
    NAME ${F3D_TEST_DOC_NAME}
    DATA ${_f3d_doc_data}
    BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/${F3D_TEST_DOC_REF_IMAGE}
    RESOLUTION 800,600
    THRESHOLD ${F3D_TEST_DOC_THRESHOLD}
    ARGS ${_f3d_test_doc_args}
    LONG_TIMEOUT
    LABELS doc
  )
endfunction()

## --up
f3d_test_doc(NAME TestDocUpY DATA DamagedHelmet.glb REF_IMAGE up_y.png ROTATE ARGS --up=y -gfx)
f3d_test_doc(NAME TestDocUpZ DATA DamagedHelmet.glb REF_IMAGE up_z.png ROTATE ARGS --up=z -gfx)

## --axis
f3d_test_doc(NAME TestDocAxisOff DATA DamagedHelmet.glb REF_IMAGE axis_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocAxisOn DATA DamagedHelmet.glb REF_IMAGE axis_on.png ROTATE ARGS -fx UI)

## --grid
f3d_test_doc(NAME TestDocGridOff DATA DamagedHelmet.glb REF_IMAGE grid_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocGridOn DATA DamagedHelmet.glb REF_IMAGE grid_on.png ROTATE ARGS -fg)

## --axes-grid
f3d_test_doc(NAME TestDocAxesGridOff DATA DamagedHelmet.glb REF_IMAGE axes_grid_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocAxesGridOn DATA DamagedHelmet.glb REF_IMAGE axes_grid_on.png ROTATE ARGS --axes-grid -f)

## --edges
f3d_test_doc(NAME TestDocEdgesOff DATA DamagedHelmet.glb REF_IMAGE edges_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocEdgesOn DATA DamagedHelmet.glb REF_IMAGE edges_on.png ROTATE ARGS -f --edges --line-width=3)

## --armature
f3d_test_doc(NAME TestDocArmatureOff DATA bristleback_dota_fan-art.glb REF_IMAGE armature_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocArmatureOn DATA bristleback_dota_fan-art.glb REF_IMAGE armature_on.png ROTATE ARGS -f --armature --line-width=4)

## --font-scale
f3d_test_doc(NAME TestDocFontScale1 DATA DamagedHelmet.glb REF_IMAGE font_scale_1.png ROTATE ARGS -nf UI)
f3d_test_doc(NAME TestDocFontScale2 DATA DamagedHelmet.glb REF_IMAGE font_scale_2.png ROTATE ARGS -nf --font-scale=2 UI)

## --font-color
f3d_test_doc(NAME TestDocFontColorDefault DATA DamagedHelmet.glb REF_IMAGE font_color_default.png ROTATE ARGS -nfm UI)
f3d_test_doc(NAME TestDocFontColorRed DATA DamagedHelmet.glb REF_IMAGE font_color_red.png ROTATE ARGS -nfm --font-color=red UI)

## --backdrop-opacity
f3d_test_doc(NAME TestDocBackdropOpacityDefault DATA DamagedHelmet.glb REF_IMAGE backdrop_opacity_default.png ROTATE ARGS -nfm UI)
f3d_test_doc(NAME TestDocBackdropOpacity02 DATA DamagedHelmet.glb REF_IMAGE backdrop_opacity_0.2.png ROTATE ARGS -nfm --backdrop-opacity=0.2 UI)

## --normal-glyphs
f3d_test_doc(NAME TestDocNormalGlyphsOff DATA DamagedHelmet.glb REF_IMAGE normal_glyphs_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocNormalGlyphsOn DATA DamagedHelmet.glb REF_IMAGE normal_glyphs_on.png ROTATE ARGS --normal-glyphs --normal-glyphs-scale=.3 -f)

## --normal-glyphs-scale
f3d_test_doc(NAME TestDocNormalGlyphsScale03 DATA DamagedHelmet.glb REF_IMAGE normal_glyphs_scale_0.3.png ROTATE ARGS --normal-glyphs --normal-glyphs-scale=.3 -f)
f3d_test_doc(NAME TestDocNormalGlyphsScale1 DATA DamagedHelmet.glb REF_IMAGE normal_glyphs_scale_1.png ROTATE ARGS --normal-glyphs --normal-glyphs-scale=1 -f)

## --point-sprites
f3d_test_doc(NAME TestDocPointSpritesNone DATA pointCloud.vtp REF_IMAGE point_sprites_none.png ARGS -s --colormap-file=${F3D_SOURCE_DIR}/resources/colormaps/viridis.png)
f3d_test_doc(NAME TestDocPointSpritesSphere DATA pointCloud.vtp REF_IMAGE point_sprites_sphere.png ARGS -s --point-sprites=sphere --point-sprites-size=5 --colormap-file=${F3D_SOURCE_DIR}/resources/colormaps/viridis.png)

## --line-width
f3d_test_doc(NAME TestDocLineWidth1 DATA DamagedHelmet.glb REF_IMAGE line_width_1.png ROTATE ARGS -f --edges --line-width=1)
f3d_test_doc(NAME TestDocLineWidth4 DATA DamagedHelmet.glb REF_IMAGE line_width_4.png ROTATE ARGS -f --edges --line-width=4)

## --backface-type
f3d_test_doc(NAME TestDocBackfaceTypeVisible DATA ${F3D_SOURCE_DIR}/testing/data/backface.vtp REF_IMAGE backface_type_visible.png ARGS -ef --line-width=4 --backface-type=visible)
f3d_test_doc(NAME TestDocBackfaceTypeHidden DATA ${F3D_SOURCE_DIR}/testing/data/backface.vtp REF_IMAGE backface_type_hidden.png ARGS -ef --line-width=4 --backface-type=hidden)

## --color
f3d_test_doc(NAME TestDocColorRed DATA dragon.vtu REF_IMAGE color_red.png ROTATE ARGS -f --tone-mapping --color=red)
f3d_test_doc(NAME TestDocColorBlue DATA dragon.vtu REF_IMAGE color_blue.png ROTATE ARGS -f --tone-mapping --color=blue)

## --opacity
f3d_test_doc(NAME TestDocOpacity1 DATA dragon.vtu REF_IMAGE opacity_1.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocOpacity02 DATA dragon.vtu REF_IMAGE opacity_0.2.png ROTATE ARGS -f --opacity=0.2 --blending=ddp)

## --roughness
f3d_test_doc(NAME TestDocRoughness02 DATA dragon.vtu REF_IMAGE roughness_0.2.png ROTATE ARGS -f --color=0.2,0.2,0.2 --roughness=0.2 --tone-mapping)
f3d_test_doc(NAME TestDocRoughness08 DATA dragon.vtu REF_IMAGE roughness_0.8.png ROTATE ARGS -f --color=0.2,0.2,0.2 --roughness=0.8 --tone-mapping)

## --metallic
f3d_test_doc(NAME TestDocMetallic0 DATA dragon.vtu REF_IMAGE metallic_0.png ROTATE ARGS -f --metallic=0 --tone-mapping)
f3d_test_doc(NAME TestDocMetallic1 DATA dragon.vtu REF_IMAGE metallic_1.png ROTATE ARGS -f --metallic=1 --tone-mapping)

## --base-ior
f3d_test_doc(NAME TestDocDefault DATA dragon.vtu REF_IMAGE base_ior_default.png ROTATE ARGS -f --color=black --tone-mapping)
f3d_test_doc(NAME TestDocIOR25 DATA dragon.vtu REF_IMAGE base_ior_2.5.png ROTATE ARGS -f --color=black --base-ior=2.5 --tone-mapping)

## --hdri-ambient
f3d_test_doc(NAME TestDocHDRIAmbientOFF DATA dragon.vtu REF_IMAGE hdri_ambient_off.png ROTATE ARGS --tone-mapping)
f3d_test_doc(NAME TestDocHDRIAmbientON DATA dragon.vtu REF_IMAGE hdri_ambient_on.png ROTATE ARGS -f --tone-mapping)

## --texture-matcap
f3d_test_doc(NAME TestDocMatcapOFF DATA DamagedHelmet.glb REF_IMAGE matcap_off.png ROTATE ARGS -f --tone-mapping)
f3d_test_doc(NAME TestDocMatcapON DATA DamagedHelmet.glb REF_IMAGE matcap_on.png ROTATE ARGS -f --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png --tone-mapping)

## --texture-base-color
f3d_test_doc(NAME TestDocTextureBaseColorOFF DATA DamagedHelmet.glb REF_IMAGE texture_base_color_off.png ROTATE ARGS -f --tone-mapping --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureBaseColorON DATA DamagedHelmet.glb REF_IMAGE texture_base_color_on.png ROTATE ARGS -f --tone-mapping --texture-material= --texture-emissive= --texture-normal=)

## --texture-material
f3d_test_doc(NAME TestDocTextureMaterialOFF DATA DamagedHelmet.glb REF_IMAGE texture_material_off.png ROTATE ARGS -f --tone-mapping --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureMaterialON DATA DamagedHelmet.glb REF_IMAGE texture_material_on.png ROTATE ARGS -f --tone-mapping --texture-base-color= --texture-emissive= --texture-normal=)

## --texture-emissive
f3d_test_doc(NAME TestDocTextureEmissiveOFF DATA DamagedHelmet.glb REF_IMAGE texture_emissive_off.png ROTATE ARGS -f --color=black --tone-mapping --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureEmissiveON DATA DamagedHelmet.glb REF_IMAGE texture_emissive_on.png ROTATE ARGS -f --color=black --tone-mapping --texture-base-color= --texture-material= --texture-normal=)

## --emissive-factor
f3d_test_doc(NAME TestDocEmissiveFactorDefault DATA DamagedHelmet.glb REF_IMAGE emissive_factor_default.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocEmissiveFactorRed DATA DamagedHelmet.glb REF_IMAGE emissive_factor_red.png ROTATE ARGS -f --emissive-factor=2,0.1,0.1)

## --texture-normal
f3d_test_doc(NAME TestDocTextureNormalOFF DATA DamagedHelmet.glb REF_IMAGE texture_normal_off.png ROTATE ARGS -f --tone-mapping --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureNormalON DATA DamagedHelmet.glb REF_IMAGE texture_normal_on.png ROTATE ARGS -f --tone-mapping --texture-base-color= --texture-material= --texture-emissive=)

## --normal-scale
f3d_test_doc(NAME TestDocNormalScaleDefault DATA DamagedHelmet.glb REF_IMAGE normal_scale_default.png ROTATE ARGS -f --tone-mapping)
f3d_test_doc(NAME TestDocNormalScale2 DATA DamagedHelmet.glb REF_IMAGE normal_scale_2.png ROTATE ARGS -f --tone-mapping --normal-scale=2)

## --textures-transform
f3d_test_doc(NAME TestDocTextureTransformDefault DATA ${F3D_SOURCE_DIR}/testing/data/cube_unlit.obj REF_IMAGE texture_transform_default.png ROTATE ARGS -e --line-width=2 --camera-position=5,2.5,5.9 --camera-focal-point=0.2,-0.2,0 --camera-view-up=-0.216,0.942,-0.258)
f3d_test_doc(NAME TestDocTextureTransformRotation DATA ${F3D_SOURCE_DIR}/testing/data/cube_unlit.obj REF_IMAGE texture_transform_rotation.png ROTATE ARGS -e --line-width=2 --textures-transform=angle:10 --camera-position=5,2.5,5.9 --camera-focal-point=0.2,-0.2,0 --camera-view-up=-0.216,0.942,-0.258)

## --checkerboard
f3d_test_doc(NAME TestDocCheckerboardOFF DATA DamagedHelmet.glb REF_IMAGE checkerboard_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocCheckerboardON DATA DamagedHelmet.glb REF_IMAGE checkerboard_on.png ROTATE ARGS -f --checkerboard)

## --unlit
f3d_test_doc(NAME TestDocUnlitOFF DATA DamagedHelmet.glb REF_IMAGE unlit_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocUnlitON DATA DamagedHelmet.glb REF_IMAGE unlit_on.png ROTATE ARGS -f --unlit)

## --background-color
f3d_test_doc(NAME TestDocBackgroundColorDefault DATA DamagedHelmet.glb REF_IMAGE background_color_default.png RESOLUTION 800,600 BACKGROUND ROTATE ARGS -f)
f3d_test_doc(NAME TestDocBackgroundColorOrange DATA DamagedHelmet.glb REF_IMAGE background_color_orange.png RESOLUTION 800,600 BACKGROUND ROTATE ARGS -f --background-color=orange)

## --fps
f3d_test_doc(NAME TestDocFPSOFF DATA DamagedHelmet.glb REF_IMAGE fps_off.png ROTATE ARGS -f UI)
f3d_test_doc(NAME TestDocFPSON DATA DamagedHelmet.glb REF_IMAGE fps_on.png ROTATE ARGS -f --fps THRESHOLD 0.08 UI)

## --filename
f3d_test_doc(NAME TestDocFilenameOFF DATA DamagedHelmet.glb REF_IMAGE filename_off.png ROTATE ARGS -f UI)
f3d_test_doc(NAME TestDocFilenameON DATA DamagedHelmet.glb REF_IMAGE filename_on.png ROTATE ARGS -f --filename UI)

## --metadata
f3d_test_doc(NAME TestDocMetadataOFF DATA DamagedHelmet.glb REF_IMAGE metadata_off.png ROTATE ARGS -f UI)
f3d_test_doc(NAME TestDocMetadataON DATA DamagedHelmet.glb REF_IMAGE metadata_on.png ROTATE ARGS -f --metadata UI)

## --scene-hierarchy
f3d_test_doc(NAME TestDocSceneHierarchyOFF DATA bristleback_dota_fan-art.glb REF_IMAGE scene_hierarchy_off.png ROTATE ARGS -f UI)
f3d_test_doc(NAME TestDocSceneHierarchyON DATA bristleback_dota_fan-art.glb REF_IMAGE scene_hierarchy_on.png ROTATE ARGS -f --scene-hierarchy UI)

## --notifications (TODO: interaction file?)

## --hdri-filename
f3d_test_doc(NAME TestDocHDRIFilenameOFF DATA DamagedHelmet.glb REF_IMAGE hdri_filename_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocHDRIFilenameON DATA DamagedHelmet.glb REF_IMAGE hdri_filename_on.png ROTATE ARGS -f --hdri-filename UI)

## --hdri-skybox
f3d_test_doc(NAME TestDocHDRISkyboxOFF DATA DamagedHelmet.glb REF_IMAGE hdri_skybox_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocHDRISkyboxON DATA DamagedHelmet.glb REF_IMAGE hdri_skybox_on.png ROTATE ARGS -fj)

## --blur-background
f3d_test_doc(NAME TestDocHDRIBlurOFF DATA DamagedHelmet.glb REF_IMAGE hdri_blur_off.png RESOLUTION 800,600 ROTATE ARGS -fj)
f3d_test_doc(NAME TestDocHDRIBlurON DATA DamagedHelmet.glb REF_IMAGE hdri_blur_on.png RESOLUTION 800,600 ROTATE ARGS -fju)

## --blur-coc
f3d_test_doc(NAME TestDocHDRICOCDefault DATA DamagedHelmet.glb REF_IMAGE hdri_coc_default.png RESOLUTION 800,600 ROTATE ARGS -fju)
f3d_test_doc(NAME TestDocHDRICOC50 DATA DamagedHelmet.glb REF_IMAGE hdri_coc_50.png RESOLUTION 800,600 ROTATE ARGS -fju --blur-coc=50)

## --light-intensity
f3d_test_doc(NAME TestDocLightIntensity1 DATA DamagedHelmet.glb REF_IMAGE light_intensity_1.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocLightIntensity05 DATA DamagedHelmet.glb REF_IMAGE light_intensity_0.5.png ROTATE ARGS --light-intensity=0.5 -f)

## --scalar-coloring
f3d_test_doc(NAME TestDocScalarColoringOFF DATA dragon.vtu REF_IMAGE scalar_coloring_off.png ROTATE ARGS)
f3d_test_doc(NAME TestDocScalarColoringON DATA dragon.vtu REF_IMAGE scalar_coloring_on.png ROTATE ARGS -s --coloring-component=2)

## --coloring-array
f3d_test_doc(NAME TestDocColoringArrayNormal DATA dragon.vtu REF_IMAGE coloring_array_normal.png ROTATE ARGS -s --coloring-array=Normals --coloring-component=2)
f3d_test_doc(NAME TestDocColoringArrayHeight DATA dragon.vtu REF_IMAGE coloring_array_height.png ROTATE ARGS -s "--coloring-array=UV coordinates from field")

## --coloring-component
f3d_test_doc(NAME TestDocColoringComponentX DATA dragon.vtu REF_IMAGE coloring_component_x.png ROTATE ARGS -s --coloring-array=Normals --coloring-component=0)
f3d_test_doc(NAME TestDocColoringComponentY DATA dragon.vtu REF_IMAGE coloring_component_y.png ROTATE ARGS -s --coloring-array=Normals --coloring-component=1)

## --coloring-by-cells
#f3d_test_doc(NAME TestDocColoringByCellsOFF DATA skull.vti REF_IMAGE coloring_by_cells_off.png ARGS -s --coloring-array=Result)
#f3d_test_doc(NAME TestDocColoringByCellsON DATA skull.vti REF_IMAGE coloring_by_cells_on.png ARGS -sc)

## --coloring-range
f3d_test_doc(NAME TestDocColoringRangeAuto DATA skull.vti REF_IMAGE coloring_range_auto.png ARGS -sv --up=z)
f3d_test_doc(NAME TestDocColoringRangeManual DATA skull.vti REF_IMAGE coloring_range_manual.png ARGS -sv --up=z --coloring-range=40,200)

## --coloring-scalar-bar
f3d_test_doc(NAME TestDocScalarBarOFF DATA skull.vti REF_IMAGE scalar_bar_off.png ARGS -sv --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocScalarBarON DATA skull.vti REF_IMAGE scalar_bar_on.png ARGS -svb --up=z --coloring-range=40,200)

## ---colormap-file
f3d_test_doc(NAME TestDocColorMapDefault DATA skull.vti REF_IMAGE color_map_default.png ARGS -sv --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocColorMapViridis DATA skull.vti REF_IMAGE color_map_viridis.png ARGS -sv --up=z --coloring-range=40,200 --colormap-file=${F3D_SOURCE_DIR}/resources/colormaps/viridis.png)

## --colormap-discretization
f3d_test_doc(NAME TestDocColorMapDiscretizationDefault DATA dragon.vtu REF_IMAGE color_map_discretization_default.png ROTATE ARGS -s "--coloring-array=UV coordinates from field")
f3d_test_doc(NAME TestDocColorMapDiscretization8 DATA dragon.vtu REF_IMAGE color_map_discretization_8.png ROTATE ARGS -s "--coloring-array=UV coordinates from field" --colormap-discretization=8)

## --volume
f3d_test_doc(NAME TestDocVolumeOFF DATA skull.vti REF_IMAGE volume_off.png ARGS -s --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocVolumeON DATA skull.vti REF_IMAGE volume_on.png ARGS -vs --up=z --coloring-range=40,200)

## --volume-opacity-file
f3d_test_doc(NAME TestDocVolumeOpacityDefault DATA skull.vti REF_IMAGE volume_opacity_default.png ARGS -vs --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocVolumeOpacityGaussian DATA skull.vti REF_IMAGE volume_opacity_gaussian.png ARGS -vs --up=z --coloring-range=40,200 --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/gaussian_opacity_map.png)

## --volume-inverse
f3d_test_doc(NAME TestDocVolumeInverseOFF DATA skull.vti REF_IMAGE volume_inverse_off.png ARGS -vs --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocVolumeInverseON DATA skull.vti REF_IMAGE volume_inverse_on.png ARGS -vs --up=z --coloring-range=40,200 --volume-inverse)

if(F3D_MODULE_RAYTRACING)
  ## --raytracing
  f3d_test_doc(NAME TestDocRaytracingOFF DATA dragon.vtu REF_IMAGE raytracing_off.png ROTATE ARGS -f --color=0.5,0.5,0.5)
  f3d_test_doc(NAME TestDocRaytracingON DATA dragon.vtu REF_IMAGE raytracing_on.png ROTATE ARGS -f --color=0.5,0.5,0.5 --raytracing --raytracing-samples=32)

  ## --raytracing-samples
  f3d_test_doc(NAME TestDocRaytracingSamples2 DATA dragon.vtu REF_IMAGE raytracing_samples_2.png ROTATE ARGS -f --color=0.5,0.5,0.5 --raytracing --raytracing-samples=2)
  f3d_test_doc(NAME TestDocRaytracingSamples6 DATA dragon.vtu REF_IMAGE raytracing_samples_6.png ROTATE ARGS -f --color=0.5,0.5,0.5 --raytracing --raytracing-samples=6)

  ## --raytracing-denoise
  f3d_test_doc(NAME TestDocRaytracingDenoiseOFF DATA dragon.vtu REF_IMAGE raytracing_denoise_off.png ROTATE ARGS -f --color=0.5,0.5,0.5 --raytracing --raytracing-samples=2)
  f3d_test_doc(NAME TestDocRaytracingDenoiseON DATA dragon.vtu REF_IMAGE raytracing_denoise_on.png ROTATE ARGS -f --color=0.5,0.5,0.5 --raytracing --raytracing-samples=2 --raytracing-denoise)
endif()

## --blending
f3d_test_doc(NAME TestDocBlendingOFF DATA dragon.vtu REF_IMAGE blending_off.png ROTATE ARGS -f --opacity=0.6)
f3d_test_doc(NAME TestDocBlendingON DATA dragon.vtu REF_IMAGE blending_on.png ROTATE ARGS -f --opacity=0.6 --blending=ddp)

## --ambient-occlusion
f3d_test_doc(NAME TestDocAmbientOcclusionOFF DATA dragon.vtu REF_IMAGE ambient_occlusion_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocAmbientOcclusionON DATA dragon.vtu REF_IMAGE ambient_occlusion_on.png ROTATE ARGS -f --ambient-occlusion)

## --anti-aliasing
f3d_test_doc(NAME TestDocAntiAliasingOFF DATA dragon.vtu REF_IMAGE anti_aliasing_off.png ROTATE ARGS -f --anti-aliasing=none)
f3d_test_doc(NAME TestDocAntiAliasingON DATA dragon.vtu REF_IMAGE anti_aliasing_on.png ROTATE ARGS -f --anti-aliasing=ssaa)

## --tone-mapping
f3d_test_doc(NAME TestDocToneMappingOFF DATA DamagedHelmet.glb REF_IMAGE tone_mapping_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocToneMappingON DATA DamagedHelmet.glb REF_IMAGE tone_mapping_on.png ROTATE ARGS -f --tone-mapping)

## --final-shader
f3d_test_doc(NAME TestDocFinalShaderOFF DATA DamagedHelmet.glb REF_IMAGE final_shader_off.png ROTATE ARGS -f)
f3d_test_doc(NAME TestDocFinalShaderGrayScale DATA DamagedHelmet.glb REF_IMAGE final_shader_gray.png ROTATE ARGS -f --verbose --final-shader "vec4 pixel(vec2 uv) { vec4 value = texture(source, uv)\\\\\\\\\\\\\\\\; float g = dot(value.rgb, vec3(0.299, 0.587, 0.114))\\\\\\\\\\\\\\\\; return vec4(vec3(g), value.a)\\\\\\\\\\\\\\\\; }")

## --display-depth
f3d_test_doc(NAME TestDocDisplayDepthOFF DATA DamagedHelmet.glb REF_IMAGE display_depth_off.png ARGS -f)
f3d_test_doc(NAME TestDocDisplayDepthON DATA DamagedHelmet.glb REF_IMAGE display_depth_on.png ARGS -fs --display-depth)
