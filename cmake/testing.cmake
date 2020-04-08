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
             ${CMAKE_SOURCE_DIR}/data/${ARGV1})
  set_tests_properties(${ARGV0} PROPERTIES
    TIMEOUT 10
    ENVIRONMENT F3D_NO_MESSAGEBOX=1)
endfunction()

f3d_test(TestPLY suzanne.ply "300,300")
f3d_test(TestOBJ suzanne.obj "300,300" "-m")
f3d_test(TestSTL suzanne.stl "300,300")
f3d_test(TestVTU dragon.vtu "300,300")
f3d_test(TestVTP cow.vtp "300,300")
f3d_test(TestVTR RectGrid2.vtr "300,300" "--scalars --roughness=1")
f3d_test(TestVTK cow.vtk "300,300")
f3d_test(TestGrid suzanne.ply "300,300" "-g")
f3d_test(TestAxis suzanne.ply "300,300" "-x")
f3d_test(TestPointCloud pointsCloud.vtp "300,300" "--point-sprites --point-size=20")
f3d_test(TestPointCloudBar pointsCloud.vtp "300,300" "-sob --point-size=20")
f3d_test(TestVRMLImporter bot2.wrl "300,300")
f3d_test(Test3DSImporter iflamigm.3ds "300,300")
f3d_test(TestOBJImporter world.obj "300,300")
f3d_test(TestScalars suzanne.ply "300,300" "--scalars=Normals --comp=1")
f3d_test(TestScalarsRange suzanne.ply "300,300" "--scalars=Normals --comp=1 --range=0,1")
f3d_test(TestScalarsWithBar suzanne.ply "300,300" "-b --scalars=Normals --comp=0")
f3d_test(TestGLTFImporter WaterBottle.glb "300,300")
f3d_test(TestMaterial suzanne.ply "300,300" "--color=0.72,0.45,0.2 --metallic=1 --roughness=0.1")
f3d_test(TestEdges suzanne.ply "300,300" "-e")
f3d_test(TestDicom IM-0001-1983.dcm "300,300" "--scalars --roughness=1")
f3d_test(TestMHD HeadMRVolume.mhd "300,300" "--scalars --roughness=1")
f3d_test(TestVTICell waveletMaterial.vti "300,300" "--scalars=Material -c --roughness=1")
f3d_test(TestSSAO suzanne.ply "300,300" "-u")
f3d_test(TestDepthPeeling suzanne.ply "300,300" "-sp --opacity=0.9")
f3d_test(TestBackground suzanne.ply "300,300" "--bg-color=0.8,0.2,0.9")
f3d_test(TestGridWithDepthPeeling suzanne.ply "300,300" "-gp --opacity 0.2")
f3d_test(TestFilename suzanne.ply "300,300" "-n")
f3d_test(TestCityGML Part-4-Buildings-V4-one.gml "300,300")
f3d_test(TestPTS samplePTS.pts "300,300")
f3d_test(TestColormap IM-0001-1983.dcm "300,300" "--scalars --roughness=1 --colormap=0,1,0,0,1,0,1,0")
f3d_test(TestCameraConfiguration suzanne.obj "300,300" "--camera-position=0,0,-10 -x --camera-view-up=1,0,0 --camera-focal-point=1,0,0 --camera-view-angle=20")
f3d_test(TestToneMapping suzanne.ply "300,300" "-a")
f3d_test(TestDepthPeelingToneMapping suzanne.ply "300,300" "--opacity=0.9 -pa")
f3d_test(TestDefaultConfigFileSimilar dragon.vtu "300,300" "-sfgxnv --progress")

if(F3D_HAS_RAYTRACING)
  f3d_test(TestOSPRayGLTF WaterBottle.glb "300,300" "-r --samples=1")
  f3d_test(TestOSPRayBackground suzanne.ply "300,300" "-r --samples=1 --bg-color=1,0,0")
  f3d_test(TestOSPRayPointCloud pointsCloud.vtp "300,300" "-r --point-sprites --point-size=20")
endif()

# Test few basic options
add_test(NAME TestHelp COMMAND $<TARGET_FILE:f3d> --help)
add_test(NAME TestVersion COMMAND $<TARGET_FILE:f3d> --version)

set_tests_properties(TestHelp TestVersion PROPERTIES
    TIMEOUT 2
    ENVIRONMENT F3D_NO_MESSAGEBOX=1)
