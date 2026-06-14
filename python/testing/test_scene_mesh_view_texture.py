import math
import numpy as np

import f3d


def test_scene_mesh_view_texture():
    """The in-memory base-color texture of a mesh_view (memory_view.base_color_texture)
    is exposed to Python and actually applied at render time. A solid-red BYTE image is
    used as an emissive base-color texture on a quad; the rendered frame must therefore
    contain strongly-red pixels (no baseline needed)."""

    engine = f3d.Engine.create(True)  # offscreen
    engine.window.size = 300, 300

    # A unit quad in the z = 0 plane with full-range texture coordinates.
    points = np.array(
        [[-1.0, -1.0, 0.0], [1.0, -1.0, 0.0], [1.0, 1.0, 0.0], [-1.0, 1.0, 0.0]],
        dtype=np.float32,
    )
    texcoords = np.array(
        [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]], dtype=np.float32
    )
    face_offsets = np.array([0, 4], dtype=np.int32)
    face_indices = np.array([0, 1, 2, 3], dtype=np.int32)

    # A 2x2 solid-red RGB texture (BYTE). Solid colour keeps the check framing-robust.
    tex = f3d.Image(2, 2, 3, f3d.Image.ChannelType.BYTE)
    tex.content = bytes([255, 0, 0] * 4)

    memory_view = f3d.MeshMemoryView()
    memory_view.points = points
    memory_view.texture_coordinates = texcoords
    memory_view.polygons_offsets = face_offsets
    memory_view.polygons_indices = face_indices
    memory_view.base_color_texture = tex
    memory_view.base_color_texture_emissive = True  # show flat/full-strength

    class TexturedQuad(f3d.MeshView):
        def get_time_range(self):
            return 0.0, 0.0

        def get_name(self):
            return "TexturedQuad"

        def get_memory_view(self, time):
            return memory_view

    engine.scene.add(TexturedQuad())

    img = engine.window.render_to_image()
    assert img.width == 300 and img.height == 300

    rgb = np.frombuffer(img.content, dtype=np.uint8).reshape(img.height, img.width, -1)
    r, g, b = rgb[..., 0].astype(int), rgb[..., 1].astype(int), rgb[..., 2].astype(int)
    strongly_red = (r > 200) & (g < 60) & (b < 60)

    # The red emissive texture must be visible: a meaningful fraction of red pixels.
    assert strongly_red.sum() > 1000, (
        f"expected the red base-color texture to render, "
        f"got {int(strongly_red.sum())} strongly-red pixels"
    )
