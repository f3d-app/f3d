## Tests the documentation illustrations in doc/user

## --up
f3d_test(NAME TestDocUpY DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/up_y.png RESOLUTION 800,600 ARGS --no-background --up=y --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -gx LABELS doc)
f3d_test(NAME TestDocUpZ DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/up_z.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -gx LABELS doc)

## --axis
f3d_test(NAME TestDocAxisOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axis_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocAxisOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axis_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -x LABELS doc)

## --grid
f3d_test(NAME TestDocGridOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/grid_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocGridOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/grid_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa -g LABELS doc)

## --axes-grid
f3d_test(NAME TestDocAxesGridOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axes_grid_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 LABELS doc)
f3d_test(NAME TestDocAxesGridOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/axes_grid_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --axes-grid LABELS doc)

## --edges
f3d_test(NAME TestDocEdgesOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/edges_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 LABELS doc)
f3d_test(NAME TestDocEdgesOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/edges_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --edges LABELS doc)

## --armature
f3d_test(NAME TestDocArmatureOff DATA RiggedFigure.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/armature_off.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=fxaa LABELS doc)
f3d_test(NAME TestDocArmatureOn DATA RiggedFigure.glb BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/armature_on.png RESOLUTION 800,600 ARGS --no-background --anti-aliasing=fxaa --armature LABELS doc)

## --font-scale
f3d_test(NAME TestDocFontScale1 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_scale_1.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -n LABELS doc)
f3d_test(NAME TestDocFontScale2 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_scale_2.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -n --font-scale=2 LABELS doc)

## --font-color
f3d_test(NAME TestDocFontColorDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_color_default.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm LABELS doc)
f3d_test(NAME TestDocFontColorRed DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/font_color_red.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm --font-color=red LABELS doc)

## --backdrop-opacity
f3d_test(NAME TestDocBackdropOpacityDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/backdrop_opacity_default.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm LABELS doc)
f3d_test(NAME TestDocBackdropOpacity02 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/backdrop_opacity_0.2.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 -nm --backdrop-opacity=0.2 LABELS doc)

## --normal-glyphs
f3d_test(NAME TestDocNormalGlyphsOff DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_off.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa LABELS doc)
f3d_test(NAME TestDocNormalGlyphsOn DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_on.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa --normal-glyphs LABELS doc)

## --normal-glyphs-scale
f3d_test(NAME TestDocNormalGlyphsScaleDefault DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_scale_default.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa --normal-glyphs LABELS doc)
f3d_test(NAME TestDocNormalGlyphsScale2 DATA f3d.vtp BASELINE_PATH ${F3D_SOURCE_DIR}/doc/user/images/normal_glyphs_scale_2.png RESOLUTION 800,600 ARGS --no-background --up=z --scalar-coloring -c --coloring-component=-2 --anti-aliasing=ssaa --normal-glyphs --normal-glyphs-scale=2 LABELS doc)
