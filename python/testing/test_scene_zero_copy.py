import tempfile
from pathlib import Path
import numpy as np
import math

import f3d


def test_scene_zero_copy():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    reference = Path(testing_dir) / "baselines/TestPythonSceneZeroCopy.png"
    output = Path(tempfile.gettempdir()) / "TestPythonSceneZeroCopy.png"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    engine.options.update(
        {
            "model.scivis.enable": True,
            "model.scivis.component": -2,
            "model.scivis.array_name": "Color",
            "model.material.base_ior": 1.0,
            "scene.animation.autoplay": True,
            "ui.animation_progress": True,
        }
    )

    points = np.array(
        [[0.0, 0.0, 0.0], [0.0, 1.0, 0.0], [1.0, 0.0, 0.0]], dtype=np.float32
    )
    face_offsets = np.array([0, 3], dtype=np.int32)
    face_indices = np.array([0, 1, 2], dtype=np.int32)
    colors = np.empty((3, 3), dtype=np.uint8)

    memory_view = f3d.MeshMemoryView()
    memory_view.points = points
    memory_view.points_time_dependent = False
    memory_view.polygons_offsets = face_offsets
    memory_view.polygons_indices = face_indices
    memory_view.polygons_time_dependent = False
    memory_view.point_scalars = {"Color": colors}
    memory_view.set_point_scalars_time_dependent("Color", False)

    class CustomMesh(f3d.MeshView):
        def get_time_range(self):
            return 0.0, 2 * math.pi

        def get_memory_view(self, time):
            c = 0.5 + math.cos(time) * 0.5
            s = 0.5 - math.sin(time + math.pi * 0.5) * 0.5
            colors[:] = [
                [255 * c, 255 * s, 255],
                [255 * s, 255, 255 * c],
                [255, 255 * c, 255 * s],
            ]
            return memory_view

    engine.scene.add(CustomMesh())

    engine.interactor.trigger_event_loop(3.14)

    img = engine.window.render_to_image()
    img.save(output)
    assert img.compare(f3d.Image(reference)) < 0.05
