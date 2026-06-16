## Tests the documentation illustrations in doc/user

## --up
f3d_test(NAME TestDocUpY DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/up_y.png RESOLUTION 800,600 ARGS --no-background --up=y --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -gx LABELS doc)
f3d_test(NAME TestDocUpZ DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/up_z.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -gx LABELS doc)

## --axis
f3d_test(NAME TestDocAxisOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axis_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocAxisOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axis_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -x LABELS doc UI)

## --grid
f3d_test(NAME TestDocGridOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/grid_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocGridOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/grid_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -g LABELS doc)

## --axes-grid
f3d_test(NAME TestDocAxesGridOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axes_grid_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 LABELS doc)
f3d_test(NAME TestDocAxesGridOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axes_grid_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --axes-grid LABELS doc  THRESHOLD 0.09) # Line rendering

## --edges
f3d_test(NAME TestDocEdgesOff DATA armor.mdl BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/edges_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -f --camera-azimuth-angle=90 LABELS doc)
f3d_test(NAME TestDocEdgesOn DATA armor.mdl BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/edges_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -f --camera-azimuth-angle=90 --edges --line-width=2 LABELS doc)

## --armature
f3d_test(NAME TestDocArmatureOff DATA RiggedFigure.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/armature_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=fxaa LABELS doc)
f3d_test(NAME TestDocArmatureOn DATA RiggedFigure.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/armature_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=fxaa --armature LABELS doc)

## --font-scale
f3d_test(NAME TestDocFontScale1 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_scale_1.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -n LABELS doc UI)
f3d_test(NAME TestDocFontScale2 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_scale_2.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -n --font-scale=2 LABELS doc UI)

## --font-color
f3d_test(NAME TestDocFontColorDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_color_default.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm LABELS doc UI)
f3d_test(NAME TestDocFontColorRed DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_color_red.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm --font-color=red LABELS doc UI)

## --backdrop-opacity
f3d_test(NAME TestDocBackdropOpacityDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/backdrop_opacity_default.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm LABELS doc UI)
f3d_test(NAME TestDocBackdropOpacity02 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/backdrop_opacity_0.2.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm --backdrop-opacity=0.2 LABELS doc UI)

## --normal-glyphs
f3d_test(NAME TestDocNormalGlyphsOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocNormalGlyphsOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa --normal-glyphs LABELS doc)

## --normal-glyphs-scale
f3d_test(NAME TestDocNormalGlyphsScaleDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_scale_default.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa --normal-glyphs LABELS doc)
f3d_test(NAME TestDocNormalGlyphsScale2 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_scale_2.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa --normal-glyphs --normal-glyphs-scale=2 LABELS doc)

## --point-sprites
f3d_test(NAME TestDocPointSpritesNone DATA bonsai_small.ply BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/point_sprites_none.png RESOLUTION 800,600 ARGS --no-background --up=-y --scalar-coloring --coloring-component=-2 LABELS doc)
f3d_test(NAME TestDocPointSpritesGaussian DATA bonsai_small.ply BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/point_sprites_gaussian.png RESOLUTION 800,600 ARGS --no-background --up=-y --scalar-coloring --coloring-component=-2 --point-sprites=gaussian --point-sprites-absolute-size --blending=sort_cpu LABELS doc)

## --line-width
f3d_test(NAME TestDocLineWidth1 DATA armor.mdl BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/line_width_1.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -f --camera-azimuth-angle=90 --edges --line-width=1 LABELS doc)
f3d_test(NAME TestDocLineWidth3 DATA armor.mdl BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/line_width_3.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -f --camera-azimuth-angle=90 --edges --line-width=3 LABELS doc)

## --backface-type (TODO: find good data)

## --color
f3d_test(NAME TestDocColorRed DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/color_red.png RESOLUTION 800,600 ARGS --no-background --color=red --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocColorBlue DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/color_blue.png RESOLUTION 800,600 ARGS --no-background --color=blue --anti-aliasing=ssaa LABELS doc)

## --opacity
f3d_test(NAME TestDocOpacity1 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/opacity_1.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocOpacity02 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/opacit_0.2.png RESOLUTION 800,600 ARGS --no-background --opacity=0.2 --blending=ddp --anti-aliasing=ssaa LABELS doc)

## --roughness
f3d_test(NAME TestDocRoughness02 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/roughness_0.2.png RESOLUTION 800,600 ARGS --no-background -f --roughness=0.2 --tone-mapping --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocRoughness08 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/roughness_0.8.png RESOLUTION 800,600 ARGS --no-background -f --roughness=0.8 --tone-mapping --anti-aliasing=ssaa LABELS doc)

## --metallic
f3d_test(NAME TestDocMetallic0 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/metallic_0.png RESOLUTION 800,600 ARGS --no-background -f --metallic=0 --tone-mapping --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocMetallic1 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/metallic_1.png RESOLUTION 800,600 ARGS --no-background -f --metallic=1 --tone-mapping --anti-aliasing=ssaa LABELS doc)

## --base-ior
f3d_test(NAME TestDocDefault DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/base_ior_default.png RESOLUTION 800,600 ARGS --no-background -f --tone-mapping --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocIOR25 DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/base_ior_2.5.png RESOLUTION 800,600 ARGS --no-background -f --base-ior=2.5 --tone-mapping --anti-aliasing=ssaa LABELS doc)

## --hdri-ambient
f3d_test(NAME TestDocHDRIAmbientOFF DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/hdri_ambient_off.png RESOLUTION 800,600 ARGS --no-background --tone-mapping --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocHDRIAmbientON DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/hdri_ambient_on.png RESOLUTION 800,600 ARGS --no-background -f --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --tone-mapping --anti-aliasing=ssaa LABELS doc)

## --texture-matcap
f3d_test(NAME TestDocMatcapOFF DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/matcap_off.png RESOLUTION 800,600 ARGS --no-background -f --tone-mapping --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocMatcapON DATA dragon.vtu BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/matcap_on.png RESOLUTION 800,600 ARGS --no-background -f --texture-matcap=${F3D_SOURCE_DIR}/testing/data/skin.png --tone-mapping --anti-aliasing=ssaa LABELS doc)

## --texture-base-color
f3d_test(NAME TestDocTextureBaseColorOFF DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_base_color_off.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-material= --texture-emissive= --texture-normal= --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocTextureBaseColorON DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_base_color_on.png RESOLUTION 800,600 ARGS --no-background --texture-material= --texture-emissive= --texture-normal= --anti-aliasing=ssaa LABELS doc)

## --texture-material
f3d_test(NAME TestDocTextureMaterialOFF DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_material_off.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-material= --texture-emissive= --texture-normal= --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocTextureMaterialON DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_material_on.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-emissive= --texture-normal= --anti-aliasing=ssaa LABELS doc)

## --texture-emissive
f3d_test(NAME TestDocTextureEmissiveOFF DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_emissive_off.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-material= --texture-emissive= --texture-normal= --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocTextureEmissiveON DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_emissive_on.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-material= --texture-normal= --anti-aliasing=ssaa LABELS doc)

## --emissive-factor
f3d_test(NAME TestDocEmissiveFactorDefault DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/emissive_factor_default.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocEmissiveFactorRed DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/emissive_factor_red.png RESOLUTION 800,600 ARGS --no-background --emissive-factor=1,0.3,0.3 --anti-aliasing=ssaa LABELS doc)

## --texture-normal
f3d_test(NAME TestDocTextureNormalOFF DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_emissive_off.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-material= --texture-emissive= --texture-normal= --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocTextureNormalON DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_emissive_on.png RESOLUTION 800,600 ARGS --no-background --texture-base-color= --texture-material= --texture-emissive= --anti-aliasing=ssaa LABELS doc)

## --normal-scale
f3d_test(NAME TestDocNormalScaleDefault DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_scale_default.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --camera-position=9.7,4.8,15.9 --camera-focal-point=1.9,2.6,2.8 --camera-view-up=-0.074,0.99,-0.125 LABELS doc)
f3d_test(NAME TestDocNormalScale2 DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_scale_2.png RESOLUTION 800,600 ARGS --no-background --normal-scale=2 --anti-aliasing=ssaa --camera-position=9.7,4.8,15.9 --camera-focal-point=1.9,2.6,2.8 --camera-view-up=-0.074,0.99,-0.125 LABELS doc)

## --textures-transform
f3d_test(NAME TestDocTextureTransformDefault DATA cube_unlit.obj BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_transform_default.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa -e --line-width=2 --camera-position=5,2.5,5.9 --camera-focal-point=0.2,-0.2,0 --camera-view-up=-0.216,0.942,-0.258 LABELS doc)
f3d_test(NAME TestDocTextureTransformRotation DATA cube_unlit.obj BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/texture_transform_rotation.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa -e --line-width=2 --textures-transform=angle:10 --camera-position=5,2.5,5.9 --camera-focal-point=0.2,-0.2,0 --camera-view-up=-0.216,0.942,-0.258 LABELS doc)

## --checkerboard
f3d_test(NAME TestDocCheckerboardOFF DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/checkerboard_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocCheckerboardON DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/checkerboard_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --checkerboard LABELS doc)

## --unlit
f3d_test(NAME TestDocUnlitOFF DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/unlit_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocUnlitON DATA Lantern/Lantern.gltf BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/unlit_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --unlit LABELS doc)

## --background-color
f3d_test(NAME TestDocBackgroundColorDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/background_color_default.png RESOLUTION 800,600 ARGS --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocBackgroundColorLime DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/background_color_lime.png RESOLUTION 800,600 ARGS --anti-aliasing=ssaa --background-color=lime LABELS doc)

## --fps
f3d_test(NAME TestDocFPSOFF DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/fps_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc UI)
f3d_test(NAME TestDocFPSON DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/fps_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --fps LABELS doc UI)

## --filename
f3d_test(NAME TestDocFilenameOFF DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/filename_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc UI)
f3d_test(NAME TestDocFilenameON DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/filename_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --filename LABELS doc UI)

## --metadata
f3d_test(NAME TestDocMetadataOFF DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/metadata_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc UI)
f3d_test(NAME TestDocMetadataON DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/metadata_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --metadata LABELS doc UI)

## --scene-hierarchy
f3d_test(NAME TestDocSceneHierarchyOFF DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/scene_hierarchy_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa LABELS doc UI)
f3d_test(NAME TestDocSceneHierarchyON DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/scene_hierarchy_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --scene-hierarchy LABELS doc UI)

## --notifications (TODO: interaction file?)

## --hdri-filename
f3d_test(NAME TestDocHDRIFilenameOFF DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/hdri_filename_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr LABELS doc UI)
f3d_test(NAME TestDocHDRIFilenameON DATA f3d.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/hdri_filename_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=ssaa --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr --hdri-filename LABELS doc UI)

