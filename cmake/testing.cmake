# F3D Testing

enable_testing()

function(f3d_test name input args)
  separate_arguments(args)
  add_test(NAME ${name}
           COMMAND ${CMAKE_BUILD_DIR}/f3d
             ${args}
             --ref ${CMAKE_SOURCE_DIR}/data/baselines/${name}.png
             ${CMAKE_SOURCE_DIR}/data/${input})
endfunction()

f3d_test(TestPLY suzanne.ply "-p --resolution 300,300")
f3d_test(TestOBJ suzanne.obj "-p --resolution 300,300")
f3d_test(TestSTL suzanne.stl "-p --resolution 300,300")
f3d_test(TestVTU dragon.vtu "-p --resolution 300,300")
f3d_test(TestGrid suzanne.ply "-pg --resolution 300,300")
f3d_test(TestAxis suzanne.ply "-px --resolution 300,300")
f3d_test(TestPointCloud pointsCloud.vtp "-p --resolution 300,300")
f3d_test(TestVRMLImporter bot2.wrl "-pi --resolution 300,300")
f3d_test(Test3DSImporter iflamigm.3ds "-pi --resolution 300,300")
f3d_test(TestOBJImporter world.obj "-pi --resolution 300,300")
f3d_test(TestScalars suzanne.ply "-pb --scalars=Normals --comp=1 --resolution 300,300")
f3d_test(TestScalarsWithBar suzanne.ply "-p --scalars=Normals --comp=0 --resolution 300,300")
if(${VTK_VERSION} VERSION_GREATER 8.2.0)
  f3d_test(TestGLTFImporter WaterBottle.glb "-pi --resolution 300,300")
endif()
