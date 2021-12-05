title: Known limitations
---

* No categorical generic field data rendering support.
* No string array categorical rendering support.
* No support for specifying manual lighting in the default scene.
* Pressing the `z` hotkey to display the FPS timer triggers a double render.
* Multiblock (.vtm, .gml) support is partial, non-surfacic data will be converted into surfaces.
* Animation support with full scene data format require VTK >= 9.0.20201016.
* Full drag and drop support require VTK >= 9.0.20210620
* `Escape` interaction events cannot be recorded.
* Drag and drop interaction cannot be recorded nor played back.
* Volume rendering and HDRI support requires a decent GPU

## Assimp

FBX, DAE, OFF, and DXF file formats rely on [Assimp](https://github.com/assimp/assimp) library. It comes with some known limitations:
- PBR materials are not supported for FBX file format
- Animations are not working very well with Assimp 5.1, it's recommended to use Assimp 5.0
- Some files can be empty, crash, or show artifacts
- DXF support is very limited: only files with polylines and 3D faces are displayed.
