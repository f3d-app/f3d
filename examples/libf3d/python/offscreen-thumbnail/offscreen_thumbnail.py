import sys
import f3d


#
# Parse optional thumbnail size:
#   argv[3]      -> square size (N x N)
#   argv[3,4]    -> width height
#
def ParseSize(argv):
    argc = len(argv)

    # Default thumbnail size
    width = 512
    height = 512

    if argc == 3:
        # Just use the default
        return True, width, height

    try:
        if argc == 4:
            # Single integer: square thumbnail
            size = int(argv[3])
            if size <= 0:
                return False, 0, 0
            width = size
            height = size
            return True, width, height

        if argc == 5:
            width = int(argv[3])
            height = int(argv[4])
            if width <= 0 or height <= 0:
                return False, 0, 0
            return True, width, height

    except Exception:
        return False, 0, 0

    return False, 0, 0


def PrintUsage(exe):
    print(
        "Usage:\n"
        f"  {exe} <input-model> <output-image> [size]\n"
        f"  {exe} <input-model> <output-image> <width> <height>\n\n"
        "Examples:\n"
        f"  {exe} model.glb thumb.png        # 512x512 (default)\n"
        f"  {exe} model.glb thumb.png 256    # 256x256\n"
        f"  {exe} model.glb thumb.png 512 256\n",
        file=sys.stderr,
    )


def main():
    try:
        argc = len(sys.argv)
        if argc < 3 or argc > 5:
            PrintUsage(sys.argv[0])
            return 1

        inputFile = sys.argv[1]
        outputFile = sys.argv[2]

        ok, width, height = ParseSize(sys.argv)
        if not ok:
            print("Invalid thumbnail size.", file=sys.stderr)
            PrintUsage(sys.argv[0])
            return 1

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

        # Add a model to the scene
        eng.scene.add(inputFile)

        # Render offscreen to an image
        eng.window.size = width, height

        # You can make the background transparent by setting no_background to True in the
        # render_to_image call.
        img = eng.window.render_to_image(False)

        # Save the resulting image
        img.save(outputFile)

        return 0

    except Exception as ex:
        print(
            "F3D thumbnail example encountered an unexpected exception:",
            file=sys.stderr,
        )
        print(ex, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
