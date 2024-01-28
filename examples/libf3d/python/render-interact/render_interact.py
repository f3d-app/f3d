import sys
import f3d

if __name__ == "__main__":
    if len(sys.argv) > 4:
        sys.exit(1)

    # Load static plugins
    f3d.Engine.autoload_plugins()

    # Create a native window engine
    eng = f3d.Engine(f3d.Window.NATIVE)

    # Load a model
    try:
        eng.loader.load_geometry(sys.argv[1])
    except RuntimeError as e:
        print(e)

    # Render
    eng.window.render()

    # Start interaction
    eng.interactor.start()
