import math
import os
from pathlib import Path
import time

import f3d


TEST_DATA_DIR = Path(__file__).parent.parent.parent.parent.parent / "testing/data"


def main():
    model_path = TEST_DATA_DIR / "suzanne.obj"
    options = {
        "scene.up-direction": "+Y",
        "render.effect.tone-mapping": True,
        "render.effect.ambient-occlusion": True,
        "render.effect.translucency-support": True,
        "render.effect.anti-aliasing": True,
    }
    anim_fps = 30
    anim_duration = 4
    orbit_count = 2

    try:
        rows, cols = os.get_terminal_size()
    except OSError:
        rows, cols = 40, 20

    # setup engine
    engine = f3d.Engine(f3d.Window.NATIVE_OFFSCREEN)
    engine.options.update(options)
    engine.loader.load_geometry(str(model_path))
    engine.window.size = rows, cols * 2

    # fit view to loaded model and grab computed camera position
    engine.window.camera.reset_to_bounds()
    x0, y0, z0 = engine.window.camera.position
    r = math.sqrt(x0**2 + z0**2)

    # loop to move the camera and render frames
    for t in realtime_animation_timesteps(anim_fps, anim_duration):
        # set camera using polar coordinates on XZ plane, and original Y altitude
        a = (t / anim_duration * orbit_count) * 2 * math.pi
        engine.window.camera.position = math.sin(a) * r, y0, math.cos(a) * r
        engine.window.camera.view_up = 0, 1, 0

        # render
        image = engine.window.render_to_image(no_background=True)
        print(image.to_terminal_text(), end="")
    print()


def realtime_animation_timesteps(fps: float, duration: float):
    """Generate times at `fps`frame/s accounting for actual frame computation time."""
    target_frame_duration = 1.0 / fps
    start_time = time.time()
    end_time = start_time + duration
    while time.time() < end_time:
        frame_start_time = time.time()

        yield frame_start_time - start_time

        actual_frame_duration = time.time() - frame_start_time
        time.sleep(max(target_frame_duration - actual_frame_duration, 0))


if __name__ == "__main__":
    main()
