import re
import sys
from argparse import ArgumentParser

import f3d


def main(argv: list[str] | None = None):
    parser = ArgumentParser()
    parser.add_argument("inputFile")
    parser.add_argument("outputFile")
    parser.add_argument("size", nargs="?", type=parse_size, default=(512, 512))

    args = parser.parse_args(argv)

    # Load static/native plugins
    f3d.Engine.autoload_plugins()

    # Create an offscreen engine
    eng = f3d.Engine.create(True)

    opt = eng.options

    # No UI overlays in thumbnails
    opt["ui.axis"] = False
    opt["ui.fps"] = False
    opt["ui.filename"] = False
    opt["ui.metadata"] = False
    opt["ui.console"] = False
    opt["ui.cheatsheet"] = False

    # Neutral background, no grid or skybox
    opt["render.grid.enable"] = False
    opt["render.background.skybox"] = False
    opt["render.background.color"] = [0.15, 0.15, 0.15]  # dark neutral gray

    # Slightly stronger lighting so assets read well at small sizes
    opt["render.light.intensity"] = 1.2

    # Post-processing: AA + AO for better thumbnails
    opt["render.effect.antialiasing.enable"] = True
    opt["render.effect.antialiasing.mode"] = "ssaa"
    opt["render.effect.ambient_occlusion"] = True

    try:
        # Add a model to the scene
        eng.scene.add(args.inputFile)
    except Exception as e:
        print(e)
        return 1

    # Render offscreen to an image
    eng.window.size = args.size[0], args.size[1]

    # You can make the background transparent by setting no_background to True in the
    # render_to_image call.
    img = eng.window.render_to_image(False)

    # Save the resulting image
    img.save(args.outputFile)

    return 0


def parse_size(size: str):
    if m := re.fullmatch(r"(\d+)([,x](\d+))?", size):
        w = int(m.group(1))
        h = int(m.group(3)) if m.group(3) else w
        return w, h
    else:
        raise ValueError(f"cannot parse size from {size!r}")


if __name__ == "__main__":
    sys.exit(main())
