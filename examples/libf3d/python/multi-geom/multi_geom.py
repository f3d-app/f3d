import sys
import f3d
from pathlib import Path

if __name__ == "__main__":
    if len(sys.argv) > 2:
        sys.exit(1)

    # Load static plugins
    f3d.Engine.autoload_plugins()

    # Create a native window engine
    eng = f3d.Engine(f3d.Window.NATIVE)

    # Load all files from provided directory as geometries
    files = [f for f in Path(sys.argv[1]).iterdir() if f.is_file()]
    for file in files:
        try:
            eng.loader.load_geometry(str(file))
        except RuntimeError as e:
            print(e)

    # Render
    eng.window.render()

    # Start interaction
    eng.interactor.start()

    sys.exit(0)
