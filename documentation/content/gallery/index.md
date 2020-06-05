title: Gallery
---

Images displayed below use public datasets, you can download them [here](https://drive.google.com/uc?export=download&id=1BVoJsyvdVIPqhUj9u5Ap9-glPLeQ_0gF).

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



<div class="gallery">
    <img src="01-f3d.png">
    <div class="desc">Raytraced CAD assembly
      <span class="code">f3d 202.vtp -xtgans -rd \-\-samples=10 \-\-range=-2,9</span>
    </div>
</div>

<div class="gallery">
    <img src="02-f3d.png">
    <div class="desc">Volume rendering of a security bag scan
      <span class="code">f3d backpack.vti -vmn \-\-range=300,1000 <br>\-\-colormap=0,0,0,0,1,1,1,1</span>
    </div>
</div>

<div class="gallery">
    <img src="03-f3d.png">
    <div class="desc">Realistic rendering #1
      <span class="code">f3d DamagedHelmet.glb \-\-hdri=lebombo_4k.hdr <br>-tuqap</span>
    </div>
</div>

<div class="gallery">
    <img src="04-f3d.png">
    <div class="desc">Showcase of interactive widgets
      <span class="code">f3d dragon.vtu -xtgans \-\-comp=0</span>
    </div>
</div>

<div class="gallery">
    <img src="05-f3d.png">
    <div class="desc">Realistic rendering #2
      <span class="code">f3d FlightHelmet.glb \-\-hdri=lebombo_4k.hdr <br>-tuqap</span>
    </div>
</div>

<div class="gallery">
    <img src="06-f3d.png">
    <div class="desc">Visualization of a CFD velocity field
      <span class="code">f3d single-pin.vtp -xtbgans <br>\-\-range=-2,8 \-\-colormap=<br>0,0.3,0.7,0,0.7,0,0.1,1,1,0.8,0.8,0</span>
    </div>
</div>

<div class="gallery">
    <img src="07-f3d.png">
    <div class="desc">Volume rendering of a medical skull scan
      <span class="code">f3d skull.vti -vxbt \-\-range=40,200</span>
    </div>
</div>

<div class="gallery">
    <img src="08-f3d.png">
    <div class="desc">Point cloud rendering using sprites
      <span class="code">f3d pointCloud.vtp -o <br>\-\-point-size=0.2 \-\-colormap=<br>0,0,0.8,0,0.4,0.9,0,0,0.8,0.9,0.9,0,1,0.5,0.5,1</span>
    </div>
</div>
