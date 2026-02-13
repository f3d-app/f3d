## Tests that needs usd plugin
## This file is only added if usd is enabled

f3d_application_test(NAME TestUSD DATA suzanne.usd PLUGIN usd)
f3d_application_test(NAME TestUSDAPrimitives DATA primitives.usda PLUGIN usd)
f3d_application_test(NAME TestUSDAPrimitivesZAxis DATA primitivesZ.usda PLUGIN usd)
f3d_application_test(NAME TestUSDAInstancing DATA instancing.usda PLUGIN usd)
f3d_application_test(NAME TestUSDAGlyphs DATA glyphs.usda PLUGIN usd)
f3d_application_test(NAME TestUSDInvalid DATA invalid.usd REGEXP "Stage failed to open" PLUGIN usd ARGS --verbose NO_BASELINE)
f3d_application_test(NAME TestUSDPurpose DATA purpose.usdc PLUGIN usd)
f3d_application_test(NAME TestUSDInterpolation DATA two_quads_interp.usda PLUGIN usd)
f3d_application_test(NAME TestUSDZMemEXR DATA small.usdz PLUGIN usd)

# This test is there to test occlusion texture and face-varying point data
# TODO: Note that the result looks incorrect because of face-varying attributes and must be fixed later
f3d_application_test(NAME TestUSDTeapot DATA Teapot.usd PLUGIN usd)

f3d_application_test(NAME TestUSDZAnimated DATA AnimatedCube.usdz PLUGIN usd ARGS --animation-time=0.3 --animation-progress THRESHOLD 0.05)
f3d_application_test(NAME TestUSDZRigged DATA RiggedSimple.usdz PLUGIN usd ARGS --animation-time=0.3)
f3d_application_test(NAME TestUSDZMaterials DATA McUsd.usdz PLUGIN usd ARGS --camera-position=1055,912,-247 --camera-focal-point=69,173,63 THRESHOLD 0.3) # High threshold because of legacy comparison methods in VTK 9.3
f3d_application_test(NAME TestUSDZMaterialsInterationReload DATA McUsd.usdz PLUGIN usd INTERACTION NO_BASELINE) # Up

if(NOT F3D_MACOS_BUNDLE)
  file(COPY "${F3D_SOURCE_DIR}/plugins/usd/configs/config.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_application_test(NAME TestDefaultConfigFileUSD DATA suzanne.usd CONFIG config_build LONG_TIMEOUT TONE_MAPPING UI LABELS "plugin;usd")
  endif()

  file(COPY "${F3D_SOURCE_DIR}/plugins/usd/configs/thumbnail.d/" DESTINATION "${CMAKE_BINARY_DIR}/share/f3d/configs/thumbnail_build.d")
  f3d_application_test(NAME TestThumbnailConfigFileUSD DATA suzanne.usd CONFIG thumbnail_build LONG_TIMEOUT TONE_MAPPING LABELS "plugin;usd")
endif()
