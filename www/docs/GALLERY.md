---
sidebar_position: 2
---

# Gallery

Examples renderings with their associated command lines.
Images and videos displayed below use public datasets, you can download them [here](https://drive.google.com/file/d/1iRh0OeJjMjjaBDLG6b_iJSkC_Jt_YQuo/view?usp=sharing).

<video src='https://user-images.githubusercontent.com/3129530/194735274-395839e9-e2cb-4a47-83b9-5c6987640032.webm' autoplay="autoplay" loop="loop" width="700"></video>

*Animated realistic rendering*: `f3d gearbox/scene.gltf --hdri-file=future_parking_2k.hdr --hdri-skybox --hdri-ambient -uqxtgas`

<video src='https://user-images.githubusercontent.com/3129530/194734947-f34bc377-8ee4-472a-b130-30ce9e86facf.webm' autoplay="autoplay" loop="loop" width="700"></video>

*Animated, skinned and morphed rendering*: `f3d dota/scene.gltf --hdri-skybox --hdri-ambient --hdri-file=future_parking_2k.hdr -uqxtgas`

<video src='https://user-images.githubusercontent.com/3129530/194735269-b62216e6-33da-484c-9f0d-39befc8253bf.webm' autoplay="autoplay" loop="loop" width="700"></video>

*Animated scientific visualization rendering*: `f3d can.ex2 -xtgans --up=+Z --scalars=VEL`

<video src='https://media.githubusercontent.com/media/f3d-app/f3d-media/main/media/counter.webm' autoplay="autoplay" loop="loop" width="700"></video>

*3D Gaussians Splatting*: `f3d counter.splat --point-size=1 --point-type=gaussian -soynxz --up=-Y --camera-position=0,1,-5.2 --camera-focal-point=0,1,0`

<img src="https://user-images.githubusercontent.com/3129530/194735272-5bcd3e7c-a333-41f5-8066-9b0bec9885e8.png" width="700" />

*Direct scalars rendering of a point cloud*: `f3d Carola_PointCloud.ply --point-size=0 --comp=-2 -so --up=+Z --hdri-skybox --hdri-ambient --hdri-file=venice_sunset_8k.hdr`

<img src="https://user-images.githubusercontent.com/3129530/194735378-b8be4df2-e030-4d13-b12f-24806cbb385b.png" width="700" />

*Raytraced CAD assembly*: `f3d 202.vtp -xtgans -rd --samples=10 --range=-2,9`

<img src="https://user-images.githubusercontent.com/3129530/194735377-127fa5d8-ece5-40c7-8176-672279ebdacc.png" width="700" />

*Volume rendering of a security bag scan*: `f3d backpack.vti -vmn --range=300,1000 --colormap=0,0,0,0,1,1,1,1`

<img src="https://user-images.githubusercontent.com/3129530/194735376-3a476643-00d6-4cfc-9a88-e0dd33658564.png" width="700" />

*Realistic rendering #1*: `f3d DamagedHelmet.glb --hdri-skybox --hdri-ambient --hdri-file=lebombo_4k.hdr -tuqap`

<img src="https://user-images.githubusercontent.com/3129530/194735416-3f386437-456c-4145-9b5e-6bb6451d7e9a.png" width="700" />

*Showcase of interactive widgets*: `f3d dragon.vtu -xtganse --comp=0`

<img src="https://user-images.githubusercontent.com/3129530/194735370-4d1f067c-704c-4118-9d94-c1adf4039c39.png" width="700" />

*Metallic Rendering of a STEP file*: `f3d eta_asm.stp –hdri=future_parking_2k.hdr -uqxtga –up=+Z –metallic=1 –roughness=0.6 –color=0.98,0.90,0.59`

<img src="https://user-images.githubusercontent.com/3129530/194735366-e79a8d45-8b76-4a65-9445-4a0ca1b63876.png" width="700" />

*Rendering of a FBX file*: `f3d zeldaPosed001.fbx –hdri=hikers_cave_2k.hdr -uqxtga`

<img src="https://user-images.githubusercontent.com/3129530/194735363-d2be23bb-90b6-4e90-941d-14b6dc24b476.png" width="700" />

*Realistic rendering #2*: `f3d FlightHelmet.glb --hdri-skybox --hdri-ambient --hdri-file=lebombo_4k.hdr -tuqap`

<img src="https://user-images.githubusercontent.com/3129530/194735361-7e827d8e-531a-4938-b608-18825a9574f8.png" width="700" />

*Visualization of a CFD velocity field*: `f3d single-pin.vtp -xtbgans --range=-2,8 --colormap=0,0.3,0.7,0,0.7,0,0.1,1,1,0.8,0.8,0`

<img src="https://user-images.githubusercontent.com/3129530/194735354-486999dd-3c57-4ab1-a9ad-b8e97bac0da0.png" width="700" />

*Volume rendering of a medical skull scan*: `f3d skull.vti -vxbt --range=40,200`

<img src="https://user-images.githubusercontent.com/3129530/194735348-7dd14922-f546-4f54-8bc2-b6602617c4cc.png" width="700" />

*Point cloud rendering using sprites*: `f3d pointCloud.vtp -o --point-size=0.2 --colormap=0,0,0.8,0,0.4,0.9,0,0,0.8,0.9,0.9,0,1,0.5,0.5,1`

## Acknowledgments

- Bristleback DOTA Fan-Art by [Nikolay_Tsys](https://sketchfab.com/Tolst).
- SY Carola (point cloud) by [Scottish Maritime Museum](https://sketchfab.com/ScottishMaritimeMuseum)
- Gearbox Animation by [DZHUSI ØNE](https://sketchfab.com/dzhusione)
- Watch movement by [Greg Brown](https://grabcad.com/greg.brown)
- Zelda - Breath Of The Wild by [theStoff](https://sketchfab.com/theStoff)
- Venice Sunset HDRI and Hiker’s Cave HDRI by [Greg Zaal](https://polyhaven.com/hdris?a=Greg%20Zaal)
- Future Parking HDRI by [Sergej Majboroda](https://polyhaven.com/hdris?a=Sergej%20Majboroda)
