# Gallery

Examples renderings with their associated command lines.
Images and videos displayed below use public datasets, you can download them [here](https://drive.google.com/file/d/1hYGAHr67T7tBrUKFrCF_uc_jnvJaQF-A/view?usp=sharing).

<video src='https://user-images.githubusercontent.com/3129530/194735274-395839e9-e2cb-4a47-83b9-5c6987640032.webm' autoplay="autoplay" loop="loop" width="700"></video>

_Animated realistic rendering_: `f3d gearbox/scene.gltf --hdri-file=future_parking_2k.hdr -fjuqxtgas`

<video src='https://user-images.githubusercontent.com/3129530/194734947-f34bc377-8ee4-472a-b130-30ce9e86facf.webm' autoplay="autoplay" loop="loop" width="700"></video>

_Animated, skinned and morphed rendering_: `f3d dota/scene.gltf --hdri-file=future_parking_2k.hdr -fjuqxtgas`

<video src='https://user-images.githubusercontent.com/3129530/194735269-b62216e6-33da-484c-9f0d-39befc8253bf.webm' autoplay="autoplay" loop="loop" width="700"></video>

_Animated scientific visualization rendering_: `f3d can.ex2 -xtgans --up=+Z --scalar-coloring --coloring-array=VEL --animation-speed-factor=0.005`

<video src='https://media.githubusercontent.com/media/f3d-app/f3d-media/main/media/counter.webm' autoplay="autoplay" loop="loop" width="700"></video>

_3D Gaussians Splatting_: `f3d counter.splat --point-sprites-size=1 --point-sprites-type=gaussian -soynxz --up=-Y --camera-position=0,1,-5.2 --camera-focal-point=0,1,0`

<img src="https://user-images.githubusercontent.com/3129530/194735272-5bcd3e7c-a333-41f5-8066-9b0bec9885e8.png" width="700" />

_Direct scalars rendering of a point cloud_: `f3d Carola_PointCloud.ply --point-size=0 --coloring-component=-2 -sofj --coloring-array=RGBA --up=+Z --hdri-file=venice_sunset_8k.hdr`

<img src="https://user-images.githubusercontent.com/3129530/194735378-b8be4df2-e030-4d13-b12f-24806cbb385b.png" width="700" />

_Raytraced CAD assembly_: `f3d 202.vtp -xtgans -rd --raytracing-samples=10 --coloring-range=-2,9`

<img src="https://user-images.githubusercontent.com/3129530/194735377-127fa5d8-ece5-40c7-8176-672279ebdacc.png" width="700" />

_Volume rendering of a security bag scan_: `f3d backpack.vti -vmn --coloring-range=300,1000 --colormap=0,#000000,1,#ffffff`

<img src="https://user-images.githubusercontent.com/3129530/194735376-3a476643-00d6-4cfc-9a88-e0dd33658564.png" width="700" />

_Realistic rendering #1_: `f3d DamagedHelmet.glb --hdri-file=lebombo_4k.hdr -fjtuqap`

<img src="https://media.githubusercontent.com/media/f3d-app/f3d-media/95b76c22d927bb24759bbe0246b6260121f2933b/media/typical.png" width="700" />

_Showcase of interactive widgets_: `f3d dragon.vtu -xtganse --coloring-component=0`

<img src="https://user-images.githubusercontent.com/3129530/194735370-4d1f067c-704c-4118-9d94-c1adf4039c39.png" width="700" />

_Metallic Rendering of a STEP file_: `f3d eta_asm.stp --hdri-file=future_parking_2k.hdr -fjuqxtga --up=+Z --metallic=1 --roughness=0.6 --color=0.98,0.90,0.59 --scalar-coloring=false`

<img src="https://user-images.githubusercontent.com/3129530/194735366-e79a8d45-8b76-4a65-9445-4a0ca1b63876.png" width="700" />

_Rendering of a FBX file_: `f3d zeldaPosed001.fbx --hdri-file=hikers_cave_2k.hdr -fjuqxtga --light-intensity=1.5`

<img src="https://user-images.githubusercontent.com/3129530/194735363-d2be23bb-90b6-4e90-941d-14b6dc24b476.png" width="700" />

_Realistic rendering #2_: `f3d FlightHelmet.glb --hdri-file=lebombo_4k.hdr -fjtuqap`

<img src="https://user-images.githubusercontent.com/3129530/194735361-7e827d8e-531a-4938-b608-18825a9574f8.png" width="700" />

_Visualization of a CFD velocity field_: `f3d single-pin.vtp -xtbgans --coloring-range=-2,8 --colormap=0,0.3,0.7,0,0.7,0,0.1,1,1,0.8,0.8,0`

<img src="https://user-images.githubusercontent.com/3129530/194735354-486999dd-3c57-4ab1-a9ad-b8e97bac0da0.png" width="700" />

_Volume rendering of a medical skull scan_: `f3d skull.vti -vxbt --coloring-range=40,200 --camera-direction=1,0,0`

<img src="https://user-images.githubusercontent.com/3129530/194735348-7dd14922-f546-4f54-8bc2-b6602617c4cc.png" width="700" />

_Point cloud rendering using sprites_: `f3d pointCloud.vtp -o --point-size=0.2 --colormap=0,0,0.8,0,0.4,0.9,0,0,0.8,0.9,0.9,0,1,0.5,0.5,1 --up=+Z`

## Acknowledgments

- Bristleback DOTA Fan-Art by [Nikolay_Tsys](https://sketchfab.com/Tolst).
- SY Carola (point cloud) by [Scottish Maritime Museum](https://sketchfab.com/ScottishMaritimeMuseum)
- Gearbox Animation by [DZHUSI ØNE](https://sketchfab.com/dzhusione)
- Watch movement by [Greg Brown](https://grabcad.com/greg.brown)
- Zelda - Breath Of The Wild by [theStoff](https://sketchfab.com/theStoff)
- Venice Sunset HDRI and Hiker’s Cave HDRI by [Greg Zaal](https://polyhaven.com/hdris?a=Greg%20Zaal)
- Future Parking HDRI by [Sergej Majboroda](https://polyhaven.com/hdris?a=Sergej%20Majboroda)
