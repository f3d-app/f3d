title: Gallery
---

Images and videos displayed below use public datasets, you can download them [here](https://drive.google.com/file/d/1iRh0OeJjMjjaBDLG6b_iJSkC_Jt_YQuo/view?usp=sharing).

<style>
  div.gallery {
      margin: 5px;
      display: inline-block;
      width: 300px;
  }

  div.gallery img {
      width: 100%;
      height: auto;
  }

  div.desc {
      text-align: center;
      height:50px;
  }
  span.code {
    font: 10px Courier New;
  }

</style>

<script src="https://cdn.jsdelivr.net/npm/jquery@3.5.1/dist/jquery.min.js"></script>
<link rel="stylesheet" href="https://cdn.jsdelivr.net/gh/fancyapps/fancybox@3.5.7/dist/jquery.fancybox.min.css" />
<script src="https://cdn.jsdelivr.net/gh/fancyapps/fancybox@3.5.7/dist/jquery.fancybox.min.js"></script>

<div class="gallery">
    <a data-fancybox="gallery" href="gearbox.webm"><video autoplay loop muted src="gearbox.webm" type="video/webm"/></a>
    <div class="desc">Animated realistic rendering
      <span class="code">f3d gearbox/scene.gltf \-\-hdri=future_parking_2k.hdr -uqxtgas</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="dota.webm"><video autoplay loop muted src="dota.webm" type="video/webm"/></a>
    <div class="desc">Animated, skinned and morphed rendering
      <span class="code">f3d dota/scene.gltf \-\-hdri=future_parking_2k.hdr -uqxtgas</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="can.webm"><video autoplay loop muted src="can.webm" type="video/webm"/></a>
    <div class="desc">Animated SciViz rendering
      <span class="code">f3d can.ex2 -xtgans \-\-up=+Z \-\-scalars=VEL</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="directScalars.png"><img src="directScalars.png"></a>
    <div class="desc">Direct scalars rendering of a point cloud
      <span class="code">f3d Carola_PointCloud.ply \-\-point-size=0 \-\-comp=-2 -so \-\-up=+Z \-\-hdri=venice_sunset_8k.hdr</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="01-f3d.png"><img src="01-f3d.png"></a>
    <div class="desc">Raytraced CAD assembly
      <span class="code">f3d 202.vtp -xtgans -rd \-\-samples=10 \-\-range=-2,9</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="02-f3d.png"><img src="02-f3d.png"></a>
    <div class="desc">Volume rendering of a security bag scan
      <span class="code">f3d backpack.vti -vmn \-\-range=300,1000 <br>\-\-colormap=0,0,0,0,1,1,1,1</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="03-f3d.png"><img src="03-f3d.png"></a>
    <div class="desc">Realistic rendering #1
      <span class="code">f3d DamagedHelmet.glb \-\-hdri=lebombo_4k.hdr <br>-tuqap</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="04-f3d.png"><img src="04-f3d.png"></a>
    <div class="desc">Showcase of interactive widgets
      <span class="code">f3d dragon.vtu -xtganse \-\-comp=0</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="brass.png"><img src="brass.png"></a>
    <div class="desc">Metallic Rendering of a STEP file
      <span class="code">f3d eta_asm.stp --hdri=future_parking_2k.hdr -uqxtga --up=+Z --metallic=1 --roughness=0.6 <br>--color=0.98,0.90,0.59</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="zelda.png"><img src="zelda.png"></a>
    <div class="desc">Rendering of a FBX file
      <span class="code">f3d zeldaPosed001.fbx --hdri=hikers_cave_2k.hdr -uqxtga</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="05-f3d.png"><img src="05-f3d.png"></a>
    <div class="desc">Realistic rendering #2
      <span class="code">f3d FlightHelmet.glb \-\-hdri=lebombo_4k.hdr <br>-tuqap</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="06-f3d.png"><img src="06-f3d.png"></a>
    <div class="desc">Visualization of a CFD velocity field
      <span class="code">f3d single-pin.vtp -xtbgans <br>\-\-range=-2,8 \-\-colormap=<br>0,0.3,0.7,0,0.7,0,0.1,1,1,0.8,0.8,0</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="07-f3d.png"><img src="07-f3d.png"></a>
    <div class="desc">Volume rendering of a medical skull scan
      <span class="code">f3d skull.vti -vxbt \-\-range=40,200</span>
    </div>
</div>

<div class="gallery">
    <a data-fancybox="gallery" href="08-f3d.png"><img src="08-f3d.png"></a>
    <div class="desc">Point cloud rendering using sprites
      <span class="code">f3d pointCloud.vtp -o <br>\-\-point-size=0.2 \-\-colormap=<br>0,0,0.8,0,0.4,0.9,0,0,0.8,0.9,0.9,0,1,0.5,0.5,1</span>
    </div>
</div>

Bristleback DOTA Fan-Art by Nikolay_Tsys
https://sketchfab.com/Tolst

SY Carola (point cloud) by Scottish Maritime Museum
https://sketchfab.com/ScottishMaritimeMuseum

Gearbox Animation by DZHUSI ØNE
https://sketchfab.com/dzhusione

Watch movement by Greg Brown
https://grabcad.com/greg.brown

Zelda - Breath Of The Wild by theStoff
https://sketchfab.com/theStoff

Venice Sunset HDRI by Greg Zaal
Hiker's Cave HDRI by Greg Zaal
https://polyhaven.com/hdris?a=Greg%20Zaal

Future Parking HDRI by Sergej Majboroda
https://polyhaven.com/hdris?a=Sergej%20Majboroda

<!-- hidden gif to be used by README.md -->
<p hidden><img src="dota.gif"></p>
