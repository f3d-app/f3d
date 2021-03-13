# F3D Testing

enable_testing()

function(f3d_test)
  separate_arguments(ARGV3)
  add_test(NAME ${ARGV0}
           COMMAND $<TARGET_FILE:f3d>
             ${ARGV3}
             --dry-run
             --resolution=${ARGV2}
             --ref ${CMAKE_SOURCE_DIR}/data/baselines/${ARGV0}.png
             --output ${CMAKE_BINARY_DIR}/Testing/Temporary/${ARGV0}.png
             ${CMAKE_SOURCE_DIR}/data/testing/${ARGV1})
  set_tests_properties(${ARGV0} PROPERTIES TIMEOUT 10)
endfunction()

function(f3d_test_no_baseline)
  separate_arguments(ARGV3)
  add_test(NAME ${ARGV0}
           COMMAND $<TARGET_FILE:f3d>
             ${ARGV3}
             --dry-run
             --resolution=${ARGV2}
             --output ${CMAKE_BINARY_DIR}/Testing/Temporary/output.png
             ${CMAKE_SOURCE_DIR}/data/testing/${ARGV1})
  set_tests_properties(${ARGV0} PROPERTIES TIMEOUT 10)
endfunction()

function(f3d_test_no_render)
  separate_arguments(ARGV2)
  add_test(NAME ${ARGV0}
           COMMAND $<TARGET_FILE:f3d>
             ${ARGV2}
             --dry-run
             --no-render
             ${CMAKE_SOURCE_DIR}/data/testing/${ARGV1})
  set_tests_properties(${ARGV0} PROPERTIES TIMEOUT 10)
endfunction()

function(f3d_test_no_data)
  separate_arguments(ARGV1)
  add_test(NAME ${ARGV0}
           COMMAND $<TARGET_FILE:f3d>
             ${ARGV1}
             --dry-run)
  set_tests_properties(${ARGV0} PROPERTIES
    TIMEOUT 10)
endfunction()

f3d_test(TestPLY suzanne.ply "300,300")
f3d_test(TestOBJ suzanne.obj "300,300" "--geometry-only")
f3d_test(TestSTL suzanne.stl "300,300")
f3d_test(TestVTU dragon.vtu "300,300")
f3d_test(TestVTP cow.vtp "300,300")
f3d_test(TestVTR RectGrid2.vtr "300,300" "--scalars --roughness=1")
f3d_test(TestVTS bluntfin.vts "300,300")
f3d_test(TestVTM mb.vtm "300,300")
f3d_test(TestVTK cow.vtk "300,300")
f3d_test(TestNRRD beach.nrrd "300,300" "-s")
f3d_test(TestGrid suzanne.ply "300,300" "-g")
f3d_test(TestAxis suzanne.ply "300,300" "-x")
f3d_test(TestPointCloud pointsCloud.vtp "300,300" "-o --point-size=20")
f3d_test(TestPointCloudBar pointsCloud.vtp "300,300" "-sob --point-size=20")
f3d_test(TestPointCloudUG pointsCloud.vtu "300,300" "-o --point-size=20")
f3d_test(TestPointCloudVolume bluntfin.vts "300,300" "-sob")
f3d_test(TestVRMLImporter bot2.wrl "300,300")
f3d_test(Test3DSImporter iflamigm.3ds "300,300" "--up=+Z")
f3d_test(TestOBJImporter world.obj "300,300")
f3d_test(TestScalars suzanne.ply "300,300" "--scalars=Normals --comp=1")
f3d_test(TestScalarsRange suzanne.ply "300,300" "--scalars=Normals --comp=1 --range=0,1")
f3d_test(TestScalarsWithBar suzanne.ply "300,300" "-b --scalars=Normals --comp=0")
f3d_test(TestGLTFImporter WaterBottle.glb "300,300")
f3d_test(TestGLTFImporterUnlit UnlitTest.glb "300,300")
f3d_test(TestGLTFImporterWithAnimation BoxAnimated.gltf "300,300")
f3d_test(TestGLTFReaderWithAnimation BoxAnimated.gltf "300,300" "--geometry-only")
f3d_test(TestMaterial suzanne.ply "300,300" "--color=0.72,0.45,0.2 --metallic=1 --roughness=0.1")
f3d_test(TestDicom IM-0001-1983.dcm "300,300" "--scalars --roughness=1")
f3d_test(TestMHD HeadMRVolume.mhd "300,300" "--scalars --roughness=1")
f3d_test(TestVTICell waveletMaterial.vti "300,300" "--scalars=Material -c --roughness=1")
f3d_test(TestSSAO suzanne.ply "300,300" "-u")
f3d_test(TestDepthPeeling suzanne.ply "300,300" "-sp --opacity=0.9")
f3d_test(TestBackground suzanne.ply "300,300" "--bg-color=0.8,0.2,0.9")
f3d_test(TestGridWithDepthPeeling suzanne.ply "300,300" "-gp --opacity 0.2")
f3d_test(TestFilename suzanne.ply "300,300" "-n")
f3d_test(TestFilenameWhiteBg suzanne.ply "300,300" "-n --bg-color=1,1,1")
f3d_test(TestCityGML Part-4-Buildings-V4-one.gml "300,300")
f3d_test(TestExodus disk_out_ref.ex2 "300,300" "-s --camera-position=-11,-2,-49")
f3d_test(TestPTS samplePTS.pts "300,300")
f3d_test(TestColormap IM-0001-1983.dcm "300,300" "--scalars --roughness=1 --colormap=0,1,0,0,1,0,1,0")
f3d_test(TestCameraConfiguration suzanne.obj "300,300" "--camera-position=0,0,-10 -x --camera-view-up=1,0,0 --camera-focal-point=1,0,0 --camera-view-angle=20")
f3d_test(TestToneMapping suzanne.ply "300,300" "-t")
f3d_test(TestDepthPeelingToneMapping suzanne.ply "300,300" "--opacity=0.9 -pt")
f3d_test(TestDefaultConfigFileSimilar dragon.vtu "300,300" "-stagxn --progress")
f3d_test(TestVolume HeadMRVolume.mhd "300,300" "-v --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1")
f3d_test(TestVolumeMag vase_4comp.vti "300,300" "-vb")
f3d_test(TestVolumeComp vase_4comp.vti "300,300" "-vb --comp=3")
f3d_test(TestVolumeDirect vase_4comp.vti "300,300" "-vb --comp=-2")
f3d_test(TestVolumeCells waveletArrays.vti "300,300" "-vb --cells")
f3d_test(TestVolumeNonScalars waveletArrays.vti "300,300" "-vb --scalars=RandomPointScalars")
f3d_test(TestTextures WaterBottle.glb "300,300" "--geometry-only --texture-material=${CMAKE_SOURCE_DIR}/data/testing/red.jpg --roughness=1 --metallic=1 --texture-base-color=${CMAKE_SOURCE_DIR}/data/testing/albedo.png --texture-normal=${CMAKE_SOURCE_DIR}/data/testing/normal.png --texture-emissive=${CMAKE_SOURCE_DIR}/data/testing/red.jpg --emissive-factor=0.1,0.1,0.1")
f3d_test(TestMetaData pdiag.vtu "300,300" "-m")
f3d_test(TestMultiblockMetaData mb.vtm "300,300" "-m")
f3d_test(TestHDRI suzanne.ply "300,300" "--hdri=${CMAKE_SOURCE_DIR}/data/testing/palermo_park_1k.hdr")
f3d_test(TestHDRIOrient suzanne.stl "300,300" "--up=+Z --hdri=${CMAKE_SOURCE_DIR}/data/testing/palermo_park_1k.hdr")
f3d_test(TestHDRIBlur suzanne.ply "300,300" "-u --hdri=${CMAKE_SOURCE_DIR}/data/testing/palermo_park_1k.hdr")
f3d_test(TestHDRIEdges suzanne.ply "300,300" "-e --hdri=${CMAKE_SOURCE_DIR}/data/testing/palermo_park_1k.hdr")
f3d_test(TestTIFF logo.tif "300,300" "-sy --up=-Y")
f3d_test(TestUTF8 "(ノಠ益ಠ )ノ.vtp" "300,300")
f3d_test(TestGenericImporterAnimation small.ex2 "300,300")
f3d_test(TestAnimationIndex InterpolationTest.glb "300,300" "--animation-index=7")
f3d_test(TestNonExistentFile nonExistentFile.vtp "300,300" "--filename")
f3d_test(TestUnsupportedFile unsupportedFile.dummy "300,300" "--filename")

if(VTK_VERSION VERSION_GREATER_EQUAL 9.0.20200527)
  f3d_test(TestEdges suzanne.ply "300,300" "-e")
  f3d_test(TestLineWidth cow.vtk "300,300" "-e --line-width=5")
endif()

if(VTK_VERSION VERSION_GREATER 9.0.20210228)
  f3d_test(TestCameraPersp Cameras.gltf "300,300" "--camera-index=0")
  f3d_test(TestCameraOrtho Cameras.gltf "300,300" "--camera-index=1")

  # Test Verbose camera
  f3d_test_no_render(TestVerboseCamera Cameras.gltf "--camera-index=1 --verbose")
  set_tests_properties(TestVerboseCamera PROPERTIES PASS_REGULAR_EXPRESSION "0:.*1:")
endif()

if(F3D_HAS_RAYTRACING)
  f3d_test(TestOSPRayGLTF WaterBottle.glb "300,300" "-r --samples=1")
  f3d_test(TestOSPRayBackground suzanne.ply "300,300" "-r --samples=1 --bg-color=1,0,0")
  f3d_test(TestOSPRayPointCloud pointsCloud.vtp "300,300" "-r --point-size=20")
endif()

## Tests to increase coverage

# Output option test
f3d_test_no_baseline(TestOutput cow.vtp "300,300")
f3d_test_no_baseline(TestOutputOutput cow.vtp "300,300" "--ref=${CMAKE_BINARY_DIR}/Testing/Temporary/output.png")

# No background option cannot be tested with reference
f3d_test_no_baseline(TestOutputNoBackground cow.vtp "300,300" "--no-background")
set_tests_properties(TestOutput TestOutputOutput TestOutputNoBackground PROPERTIES RUN_SERIAL TRUE)

# Simple verbosity test
f3d_test_no_render(TestVerbose dragon.vtu "-s --verbose")
set_tests_properties(TestVerbose PROPERTIES PASS_REGULAR_EXPRESSION "Using first found array: Normals.*Number of points: 69827\nNumber of cells: 139650")

# Unknow scalar array verbosity test
f3d_test_no_render(TestVerboseWrongArray dragon.vtu "--scalars=dummy --verbose")
set_tests_properties(TestVerboseWrongArray PROPERTIES PASS_REGULAR_EXPRESSION "Unknow scalar array: dummy")

# Default scalar array verbosity test
f3d_test_no_render(TestVerboseDefaultScalar HeadMRVolume.mhd "-s --verbose")
set_tests_properties(TestVerboseDefaultScalar PROPERTIES PASS_REGULAR_EXPRESSION "Using default scalar array: MetaImage")

# Incorrect component test
f3d_test_no_baseline(TestIncorrectComponent dragon.vtu "300,300"  "-s --comp=4")
set_tests_properties(TestIncorrectComponent PROPERTIES PASS_REGULAR_EXPRESSION "Invalid component index: 4")

# Incorrect range test
f3d_test_no_baseline(TestIncorrectRange dragon.vtu "300,300" "-s --range=0")
set_tests_properties(TestIncorrectRange PROPERTIES PASS_REGULAR_EXPRESSION "The range specified does not have exactly 2 values, using automatic range.")

# Incorrect color map
f3d_test_no_baseline(TestIncorrectColormap IM-0001-1983.dcm "300,300" "--scalars --roughness=1 --colormap=0,1,0,0,1,0,1")
set_tests_properties(TestIncorrectColormap PROPERTIES PASS_REGULAR_EXPRESSION "Specified color map list count is not a multiple of 4, ignoring it.")

# Test opening a directory
f3d_test_no_render(TestDirectory mb)
set_tests_properties(TestDirectory PROPERTIES PASS_REGULAR_EXPRESSION "Loading: .*mb_._0.vt.")

# Test Verbose animation, no baseline needed
f3d_test_no_baseline(TestVerboseAnimation InterpolationTest.glb "300,300" "--verbose")
set_tests_properties(TestVerboseAnimation PROPERTIES PASS_REGULAR_EXPRESSION "7: CubicSpline Translation")

# Test Generic Importer Verbose animation
f3d_test_no_baseline(TestVerboseGenericImporterAnimation small.ex2 "300,300" "--verbose")
set_tests_properties(TestVerboseGenericImporterAnimation PROPERTIES PASS_REGULAR_EXPRESSION "0: default")

# Test Animation index out of domain error
f3d_test_no_baseline(TestVerboseAnimationIndexError1 InterpolationTest.glb "300,300" "--animation-index=48")
set_tests_properties(TestVerboseAnimationIndexError1 PROPERTIES PASS_REGULAR_EXPRESSION "Specified animation index is greater than the highest possible animation index, enabling all animations.")

# Test Animation invalied index
f3d_test_no_baseline(TestVerboseAnimationIndexError2 cow.vtp "300,300" "--animation-index=1 --verbose")
set_tests_properties(TestVerboseAnimationIndexError2 PROPERTIES PASS_REGULAR_EXPRESSION "An animation index has been specified but there are no animations available.")

# Test non existent file, do not create nonExistentFile.vtp
f3d_test_no_render(TestVerboseNonExistentFile nonExistentFile.vtp "--filename --verbose")
set_tests_properties(TestVerboseNonExistentFile PROPERTIES PASS_REGULAR_EXPRESSION "File .*/data/testing/nonExistentFile.vtp does not exist")

# Test non supported file, do not add support for .dummy file.
f3d_test_no_render(TestVerboseUnsupportedFile unsupportedFile.dummy "--filename")
set_tests_properties(TestVerboseUnsupportedFile PROPERTIES PASS_REGULAR_EXPRESSION ".*/data/testing/unsupportedFile.dummy is not a file of a supported file format")

# Test help display
f3d_test_no_data(TestHelp "--help")
set_tests_properties(TestHelp PROPERTIES PASS_REGULAR_EXPRESSION "Usage:")

# Test version display
f3d_test_no_data(TestVersion "--version")
set_tests_properties(TestVersion PROPERTIES PASS_REGULAR_EXPRESSION "Version:")

# Test that f3d can try to read config file
add_test(NAME TestNoDryRun COMMAND $<TARGET_FILE:f3d> --no-render)
set_tests_properties(TestNoDryRun PROPERTIES TIMEOUT 2)

# Test failure without a reference, please do not create a TestNoRef.png file
f3d_test(TestNoRef cow.vtp "300,300")
set_tests_properties(TestNoRef PROPERTIES WILL_FAIL TRUE)

# Test failure without a reference and  without an output, please do not create a TestNoRef.png file
add_test(NAME TestNoRefNoOutput COMMAND $<TARGET_FILE:f3d> --dry-run --resolution=300,300 --ref ${CMAKE_SOURCE_DIR}/data/baselines/TestNoRef.png ${CMAKE_SOURCE_DIR}/data/baselines/cow.vtp)
set_tests_properties(TestNoRefNoOutput PROPERTIES PASS_REGULAR_EXPRESSION "Reference image does not exists, use the --output option to output current rendering into an image file.")
set_tests_properties(TestNoRefNoOutput PROPERTIES TIMEOUT 10)

# Test failure with a bad reference, please do not create a good TestBadRef.png file
f3d_test(TestBadRef cow.vtp "300,300")
set_tests_properties(TestBadRef PROPERTIES WILL_FAIL TRUE)

# Test failure with a bed reference without an output, please do not create a good TestBadRef.png file
add_test(NAME TestBadRefNoOutput COMMAND $<TARGET_FILE:f3d> --dry-run --resolution=300,300 --ref ${CMAKE_SOURCE_DIR}/data/baselines/TestBadRef.png ${CMAKE_SOURCE_DIR}/data/baselines/cow.vtp)
set_tests_properties(TestBadRefNoOutput PROPERTIES PASS_REGULAR_EXPRESSION "Use the --output option to be able to output current rendering and diff images into files.")
set_tests_properties(TestBadRefNoOutput PROPERTIES TIMEOUT 10)
