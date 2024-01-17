import sys
import f3d

if __name__ == "__main__":
    if len(sys.argv) > 4:
        sys.exit(1)

    try:
        f3d.Engine.autoload_plugins()

        eng = f3d.Engine(f3d.Window.NATIVE_OFFSCREEN)
        eng.loader.load_geometry(sys.argv[1])

        eng.window.size = 300, 300
        img = eng.window.render_to_image()

        img.save(sys.argv[2])
    except Exception as e:
        print("F3D encountered an unexpected exception:")
        print(e)
        sys.exit(1)
