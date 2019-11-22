# F3D Testing

enable_testing()

function(f3d_test)
  separate_arguments(ARGV3)
  add_test(NAME ${ARGV0}
           COMMAND $<TARGET_FILE:f3d>
             ${ARGV3}
             --resolution ${ARGV2}
             --ref ${CMAKE_SOURCE_DIR}/data/baselines/${ARGV0}.png
             ${CMAKE_SOURCE_DIR}/data/${ARGV1})
endfunction()

f3d_test(TestPLY suzanne.ply "300,300")
f3d_test(TestOBJ suzanne.obj "300,300" "-m")
f3d_test(TestSTL suzanne.stl "300,300")
f3d_test(TestVTU dragon.vtu "300,300")
f3d_test(TestGrid suzanne.ply "300,300" "-g")
f3d_test(TestAxis suzanne.ply "300,300" "-x")
f3d_test(TestPointCloud pointsCloud.vtp "300,300" "--point-size=20")
f3d_test(TestVRMLImporter bot2.wrl "300,300")
f3d_test(Test3DSImporter iflamigm.3ds "300,300")
f3d_test(TestOBJImporter world.obj "300,300")
f3d_test(TestScalars suzanne.ply "300,300" "--scalars=Normals --comp=1")
f3d_test(TestScalarsWithBar suzanne.ply "300,300" "-b --scalars=Normals --comp=0")
f3d_test(TestGLTFImporter WaterBottle.glb "300,300")
f3d_test(TestMaterial suzanne.ply "300,300" "--color=0.72,0.45,0.2 --metallic=1 --roughness=0.1")
f3d_test(TestEdges suzanne.ply "300,300" "-e")

if(F3D_HAS_RAYTRACING)
  f3d_test(TestOSPRayGLTF WaterBottle.glb "300,300" "-r --samples=1")
endif()
