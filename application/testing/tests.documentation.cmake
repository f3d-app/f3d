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

## Wrapper that sets many default arguments for documentation illustration tests
function(f3d_test_doc)
  cmake_parse_arguments(F3D_TEST_DOC "UI" "NAME;DATA;OUT_IMAGE" "ARGS" ${ARGN})
  f3d_test(
    NAME ${F3D_TEST_DOC_NAME}
    DATA ${F3D_TEST_DOC_DATA}
    DATA_ROOT ${CMAKE_CURRENT_BINARY_DIR}
    BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/${F3D_TEST_DOC_OUT_IMAGE}
    RESOLUTION 800,600
    ARGS ${F3D_TEST_DOC_ARGS}
        --no-background
        --anti-aliasing=ssaa
        --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr
        --camera-direction=-1,-0.5,-1
    LONG_TIMEOUT
    LABELS doc
  )
endfunction()

## --up
f3d_test_doc(NAME TestDocUpY DATA DamagedHelmet.glb OUT_IMAGE up_y.png ARGS --up=y -gfx)
f3d_test_doc(NAME TestDocUpZ DATA DamagedHelmet.glb OUT_IMAGE up_z.png ARGS --up=z -gfx)

## --axis
f3d_test_doc(NAME TestDocAxisOff DATA DamagedHelmet.glb OUT_IMAGE axis_off.png ARGS -f)
f3d_test_doc(NAME TestDocAxisOn DATA DamagedHelmet.glb OUT_IMAGE axis_on.png ARGS -fx UI)

## --grid
f3d_test_doc(NAME TestDocGridOff DATA DamagedHelmet.glb OUT_IMAGE grid_off.png ARGS -f)
f3d_test_doc(NAME TestDocGridOn DATA DamagedHelmet.glb OUT_IMAGE grid_on.png ARGS -fg)

## --axes-grid
f3d_test_doc(NAME TestDocAxesGridOff DATA DamagedHelmet.glb OUT_IMAGE axes_grid_off.png ARGS -f)
f3d_test_doc(NAME TestDocAxesGridOn DATA DamagedHelmet.glb OUT_IMAGE axes_grid_on.png ARGS --axes-grid -f)

## --edges
f3d_test_doc(NAME TestDocEdgesOff DATA DamagedHelmet.glb OUT_IMAGE edges_off.png ARGS -f)
f3d_test_doc(NAME TestDocEdgesOn DATA DamagedHelmet.glb OUT_IMAGE edges_on.png ARGS -f --edges --line-width=2)

## --armature
f3d_test_doc(NAME TestDocArmatureOff DATA bristleback_dota_fan-art.glb OUT_IMAGE armature_off.png ARGS)
f3d_test_doc(NAME TestDocArmatureOn DATA bristleback_dota_fan-art.glb OUT_IMAGE armature_on.png ARGS --armature --line-width=4)

## --font-scale
f3d_test_doc(NAME TestDocFontScale1 DATA DamagedHelmet.glb OUT_IMAGE font_scale_1.png ARGS -nf UI)
f3d_test_doc(NAME TestDocFontScale2 DATA DamagedHelmet.glb OUT_IMAGE font_scale_2.png ARGS -nf --font-scale=2 UI)

## --font-color
f3d_test_doc(NAME TestDocFontColorDefault DATA DamagedHelmet.glb OUT_IMAGE font_color_default.png ARGS -nfm UI)
f3d_test_doc(NAME TestDocFontColorRed DATA DamagedHelmet.glb OUT_IMAGE font_color_red.png ARGS -nfm --font-color=red UI)

## --backdrop-opacity
f3d_test_doc(NAME TestDocBackdropOpacityDefault DATA DamagedHelmet.glb OUT_IMAGE backdrop_opacity_default.png ARGS -nfm UI)
f3d_test_doc(NAME TestDocBackdropOpacity02 DATA DamagedHelmet.glb OUT_IMAGE backdrop_opacity_0.2.png ARGS -nfm --backdrop-opacity=0.2 UI)

## --normal-glyphs
f3d_test_doc(NAME TestDocNormalGlyphsOff DATA DamagedHelmet.glb OUT_IMAGE normal_glyphs_off.png ARGS -f)
f3d_test_doc(NAME TestDocNormalGlyphsOn DATA DamagedHelmet.glb OUT_IMAGE normal_glyphs_on.png ARGS --normal-glyphs -f)

## --normal-glyphs-scale
f3d_test_doc(NAME TestDocNormalGlyphsScaleDefault DATA DamagedHelmet.glb OUT_IMAGE normal_glyphs_scale_default.png ARGS --normal-glyphs -f)
f3d_test_doc(NAME TestDocNormalGlyphsScale2 DATA DamagedHelmet.glb OUT_IMAGE normal_glyphs_scale_2.png ARGS --normal-glyphs --normal-glyphs-scale=2 -f)

## --point-sprites
#f3d_test_doc(NAME TestDocPointSpritesNone DATA bonsai_small.ply OUT_IMAGE point_sprites_none.png ARGS --up=-y --scalar-coloring --coloring-component=-2)
#f3d_test_doc(NAME TestDocPointSpritesGaussian DATA bonsai_small.ply OUT_IMAGE point_sprites_gaussian.png ARGS --up=-y --scalar-coloring --coloring-component=-2 --point-sprites=gaussian --point-sprites-absolute-size --blending=sort_cpu)

## --line-width
f3d_test_doc(NAME TestDocLineWidth1 DATA DamagedHelmet.glb OUT_IMAGE line_width_1.png ARGS -f --edges --line-width=1)
f3d_test_doc(NAME TestDocLineWidth3 DATA DamagedHelmet.glb OUT_IMAGE line_width_3.png ARGS -f --edges --line-width=3)

## --backface-type (TODO: find good data)

## --color
f3d_test_doc(NAME TestDocColorRed DATA dragon.vtu OUT_IMAGE color_red.png ARGS -f --color=red)
f3d_test_doc(NAME TestDocColorBlue DATA dragon.vtu OUT_IMAGE color_blue.png ARGS -f --color=blue)

## --opacity
f3d_test_doc(NAME TestDocOpacity1 DATA dragon.vtu OUT_IMAGE opacity_1.png ARGS -f)
f3d_test_doc(NAME TestDocOpacity02 DATA dragon.vtu OUT_IMAGE opacity_0.2.png ARGS -f --opacity=0.2 --blending=ddp)

## --roughness
f3d_test_doc(NAME TestDocRoughness02 DATA dragon.vtu OUT_IMAGE roughness_0.2.png ARGS -f --roughness=0.2 --tone-mapping)
f3d_test_doc(NAME TestDocRoughness08 DATA dragon.vtu OUT_IMAGE roughness_0.8.png ARGS -f --roughness=0.8 --tone-mapping)

## --metallic
f3d_test_doc(NAME TestDocMetallic0 DATA dragon.vtu OUT_IMAGE metallic_0.png ARGS -f --metallic=0 --tone-mapping)
f3d_test_doc(NAME TestDocMetallic1 DATA dragon.vtu OUT_IMAGE metallic_1.png ARGS -f --metallic=1 --tone-mapping)

## --base-ior
f3d_test_doc(NAME TestDocDefault DATA dragon.vtu OUT_IMAGE base_ior_default.png ARGS -f --color=black --tone-mapping)
f3d_test_doc(NAME TestDocIOR25 DATA dragon.vtu OUT_IMAGE base_ior_2.5.png ARGS -f --color=black --base-ior=2.5 --tone-mapping)

## --hdri-ambient
f3d_test_doc(NAME TestDocHDRIAmbientOFF DATA dragon.vtu OUT_IMAGE hdri_ambient_off.png ARGS --tone-mapping)
f3d_test_doc(NAME TestDocHDRIAmbientON DATA dragon.vtu OUT_IMAGE hdri_ambient_on.png ARGS -f --tone-mapping)

## --texture-matcap
f3d_test_doc(NAME TestDocMatcapOFF DATA dragon.vtu OUT_IMAGE matcap_off.png ARGS -f --tone-mapping)
f3d_test_doc(NAME TestDocMatcapON DATA dragon.vtu OUT_IMAGE matcap_on.png ARGS -f --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png --tone-mapping)

## --texture-base-color
f3d_test_doc(NAME TestDocTextureBaseColorOFF DATA DamagedHelmet.glb OUT_IMAGE texture_base_color_off.png ARGS -f --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureBaseColorON DATA DamagedHelmet.glb OUT_IMAGE texture_base_color_on.png ARGS -f --texture-material= --texture-emissive= --texture-normal=)

## --texture-material
f3d_test_doc(NAME TestDocTextureMaterialOFF DATA DamagedHelmet.glb OUT_IMAGE texture_material_off.png ARGS -f --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureMaterialON DATA DamagedHelmet.glb OUT_IMAGE texture_material_on.png ARGS -f --texture-base-color= --texture-emissive= --texture-normal=)

## --texture-emissive
f3d_test_doc(NAME TestDocTextureEmissiveOFF DATA DamagedHelmet.glb OUT_IMAGE texture_emissive_off.png ARGS -f --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureEmissiveON DATA DamagedHelmet.glb OUT_IMAGE texture_emissive_on.png ARGS -f --texture-base-color= --texture-material= --texture-normal=)

## --emissive-factor
f3d_test_doc(NAME TestDocEmissiveFactorDefault DATA DamagedHelmet.glb OUT_IMAGE emissive_factor_default.png ARGS -f)
f3d_test_doc(NAME TestDocEmissiveFactorRed DATA DamagedHelmet.glb OUT_IMAGE emissive_factor_red.png ARGS -f --emissive-factor=1,0.3,0.3)

## --texture-normal
f3d_test_doc(NAME TestDocTextureNormalOFF DATA DamagedHelmet.glb OUT_IMAGE texture_normal_off.png ARGS -f --texture-base-color= --texture-material= --texture-emissive= --texture-normal=)
f3d_test_doc(NAME TestDocTextureNormalON DATA DamagedHelmet.glb OUT_IMAGE texture_normal_on.png ARGS -f --texture-base-color= --texture-material= --texture-emissive=)

## --normal-scale
f3d_test_doc(NAME TestDocNormalScaleDefault DATA DamagedHelmet.glb OUT_IMAGE normal_scale_default.png ARGS -f)
f3d_test_doc(NAME TestDocNormalScale2 DATA DamagedHelmet.glb OUT_IMAGE normal_scale_2.png ARGS -f --normal-scale=2)

## --textures-transform
#f3d_test_doc(NAME TestDocTextureTransformDefault DATA cube_unlit.obj OUT_IMAGE texture_transform_default.png ARGS -e --line-width=2 --camera-position=5,2.5,5.9 --camera-focal-point=0.2,-0.2,0 --camera-view-up=-0.216,0.942,-0.258)
#f3d_test_doc(NAME TestDocTextureTransformRotation DATA cube_unlit.obj OUT_IMAGE texture_transform_rotation.png ARGS -e --line-width=2 --textures-transform=angle:10 --camera-position=5,2.5,5.9 --camera-focal-point=0.2,-0.2,0 --camera-view-up=-0.216,0.942,-0.258)

## --checkerboard
f3d_test_doc(NAME TestDocCheckerboardOFF DATA DamagedHelmet.glb OUT_IMAGE checkerboard_off.png ARGS -f)
f3d_test_doc(NAME TestDocCheckerboardON DATA DamagedHelmet.glb OUT_IMAGE checkerboard_on.png ARGS -f --checkerboard)

## --unlit
f3d_test_doc(NAME TestDocUnlitOFF DATA DamagedHelmet.glb OUT_IMAGE unlit_off.png ARGS -f)
f3d_test_doc(NAME TestDocUnlitON DATA DamagedHelmet.glb OUT_IMAGE unlit_on.png ARGS -f --unlit)

## --background-color
f3d_test_doc(NAME TestDocBackgroundColorDefault DATA DamagedHelmet.glb OUT_IMAGE background_color_default.png RESOLUTION 800,600 ARGS -f)
f3d_test_doc(NAME TestDocBackgroundColorOrange DATA DamagedHelmet.glb OUT_IMAGE background_color_orange.png RESOLUTION 800,600 ARGS -f --background-color=orange)

## --fps
f3d_test_doc(NAME TestDocFPSOFF DATA DamagedHelmet.glb OUT_IMAGE fps_off.png ARGS -f UI)
f3d_test_doc(NAME TestDocFPSON DATA DamagedHelmet.glb OUT_IMAGE fps_on.png ARGS -f --fps UI)

## --filename
f3d_test_doc(NAME TestDocFilenameOFF DATA DamagedHelmet.glb OUT_IMAGE filename_off.png ARGS -f UI)
f3d_test_doc(NAME TestDocFilenameON DATA DamagedHelmet.glb OUT_IMAGE filename_on.png ARGS -f --filename UI)

## --metadata
f3d_test_doc(NAME TestDocMetadataOFF DATA DamagedHelmet.glb OUT_IMAGE metadata_off.png ARGS -f UI)
f3d_test_doc(NAME TestDocMetadataON DATA DamagedHelmet.glb OUT_IMAGE metadata_on.png ARGS -f --metadata UI)

## --scene-hierarchy
f3d_test_doc(NAME TestDocSceneHierarchyOFF DATA DamagedHelmet.glb OUT_IMAGE scene_hierarchy_off.png ARGS -f UI)
f3d_test_doc(NAME TestDocSceneHierarchyON DATA DamagedHelmet.glb OUT_IMAGE scene_hierarchy_on.png ARGS -f --scene-hierarchy UI)

## --notifications (TODO: interaction file?)

## --hdri-filename
f3d_test_doc(NAME TestDocHDRIFilenameOFF DATA DamagedHelmet.glb OUT_IMAGE hdri_filename_off.png ARGS -f --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr UI)
f3d_test_doc(NAME TestDocHDRIFilenameON DATA DamagedHelmet.glb OUT_IMAGE hdri_filename_on.png ARGS -f --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr --hdri-filename UI)

## --hdri-skybox
f3d_test_doc(NAME TestDocHDRISkyboxOFF DATA DamagedHelmet.glb OUT_IMAGE hdri_skybox_off.png ARGS --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr -f)
f3d_test_doc(NAME TestDocHDRISkyboxON DATA DamagedHelmet.glb OUT_IMAGE hdri_skybox_on.png ARGS --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr -fj)

## --blur-background
f3d_test_doc(NAME TestDocHDRIBlurOFF DATA DamagedHelmet.glb OUT_IMAGE hdri_blur_off.png RESOLUTION 800,600 ARGS --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr -fj)
f3d_test_doc(NAME TestDocHDRIBlurON DATA DamagedHelmet.glb OUT_IMAGE hdri_blur_on.png RESOLUTION 800,600 ARGS --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr -fju)

## --blur-coc
f3d_test_doc(NAME TestDocHDRICOCDefault DATA DamagedHelmet.glb OUT_IMAGE hdri_coc_default.png RESOLUTION 800,600 ARGS --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr -fju)
f3d_test_doc(NAME TestDocHDRICOC50 DATA DamagedHelmet.glb OUT_IMAGE hdri_coc_50.png RESOLUTION 800,600 ARGS --hdri-file=${CMAKE_CURRENT_BINARY_DIR}/future_parking_2k.hdr -fju --blur-coc=50)

## --light-intensity
f3d_test_doc(NAME TestDocLightIntensity1 DATA DamagedHelmet.glb OUT_IMAGE light_intensity_1.png ARGS -j)
f3d_test_doc(NAME TestDocLightIntensity05 DATA DamagedHelmet.glb OUT_IMAGE light_intensity_0.5.png ARGS --light-intensity=0.5 -j)

## --scalar-coloring
f3d_test_doc(NAME TestDocScalarColoringOFF DATA dragon.vtu OUT_IMAGE scalar_coloring_off.png ARGS)
f3d_test_doc(NAME TestDocScalarColoringON DATA dragon.vtu OUT_IMAGE scalar_coloring_on.png ARGS -s --coloring-component=2)

## --coloring-array
f3d_test_doc(NAME TestDocColoringArrayNormal DATA dragon.vtu OUT_IMAGE coloring_array_normal.png ARGS -s --coloring-array=Normals --coloring-component=2)
f3d_test_doc(NAME TestDocColoringArrayHeight DATA dragon.vtu OUT_IMAGE coloring_array_height.png ARGS -s --coloring-array="UV coordinates from field")

## --coloring-component
f3d_test_doc(NAME TestDocColoringComponentX DATA dragon.vtu OUT_IMAGE coloring_component_x.png ARGS -s --coloring-array=Normals --coloring-component=0)
f3d_test_doc(NAME TestDocColoringComponentY DATA dragon.vtu OUT_IMAGE coloring_component_y.png ARGS -s --coloring-array=Normals --coloring-component=1)

## --coloring-by-cells
#f3d_test_doc(NAME TestDocColoringByCellsOFF DATA skull.vti OUT_IMAGE coloring_by_cells_off.png ARGS -s --coloring-array=Result)
#f3d_test_doc(NAME TestDocColoringByCellsON DATA skull.vti OUT_IMAGE coloring_by_cells_on.png ARGS -sc)

## --coloring-range
f3d_test_doc(NAME TestDocColoringRangeAuto DATA skull.vti OUT_IMAGE coloring_range_auto.png ARGS -sv --up=z)
f3d_test_doc(NAME TestDocColoringRangeManual DATA skull.vti OUT_IMAGE coloring_range_manual.png ARGS -sv --up=z --coloring-range=40,200)

## --coloring-scalar-bar
f3d_test_doc(NAME TestDocScalarBarOFF DATA skull.vti OUT_IMAGE scalar_bar_off.png ARGS -sv --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocScalarBarON DATA skull.vti OUT_IMAGE scalar_bar_on.png ARGS -svb --up=z --coloring-range=40,200)

## ---colormap-file
f3d_test_doc(NAME TestDocColorMapDefault DATA skull.vti OUT_IMAGE color_map_default.png ARGS -sv --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocColorMapViridis DATA skull.vti OUT_IMAGE color_map_viridis.png ARGS -sv --up=z --coloring-range=40,200 --colormap-file=${F3D_SOURCE_DIR}/resources/colormaps/viridis.png)

## --colormap-discretization
f3d_test_doc(NAME TestDocColorMapDiscretizationDefault DATA skull.vti OUT_IMAGE color_map_discretization_default.png ARGS -sv --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocColorMapDiscretization4 DATA skull.vti OUT_IMAGE color_map_discretization_4.png ARGS -sv --up=z --coloring-range=40,200 --colormap-discretization=4)

## --volume
f3d_test_doc(NAME TestDocVolumeOFF DATA skull.vti OUT_IMAGE volume_off.png ARGS -s --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocVolumeON DATA skull.vti OUT_IMAGE volume_on.png ARGS -vs --up=z --coloring-range=40,200)

## --volume-opacity-file
f3d_test_doc(NAME TestDocVolumeOpacityDefault DATA skull.vti OUT_IMAGE volume_opacity_default.png ARGS -vs --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocVolumeOpacityGaussian DATA skull.vti OUT_IMAGE volume_opacity_gaussian.png ARGS -vs --up=z --coloring-range=40,200 --volume-opacity-file=${F3D_SOURCE_DIR}/testing/data/gaussian_opacity_map.png)

## --volume-inverse
f3d_test_doc(NAME TestDocVolumeInverseOFF DATA skull.vti OUT_IMAGE volume_inverse_off.png ARGS -vs --up=z --coloring-range=40,200)
f3d_test_doc(NAME TestDocVolumeInverseON DATA skull.vti OUT_IMAGE volume_inverse_on.png ARGS -vs --up=z --coloring-range=40,200 --volume-inverse)
