import sys
from argparse import ArgumentParser
from pathlib import Path
from tempfile import gettempdir

import f3d

TEST_DATA_DIR = Path(__file__).parent.parent.parent.parent.parent / "testing/data"

if __name__ == "__main__":
    argparser = ArgumentParser()
    argparser.add_argument("models", nargs="*")
    argparser.add_argument("--into", default=gettempdir())

    args = argparser.parse_args()

    input_paths = args.models or [TEST_DATA_DIR / "suzanne.obj"]
    output_path = Path(args.into) / "f3d.png"

    try:
        eng = f3d.Engine.create(True)
        eng.window.size = 300, 300

        eng.scene.add(input_paths)

        img = eng.window.render_to_image()
        img.save(output_path)

        print(f"rendered to {output_path}")
    except Exception as e:
        print("F3D encountered an unexpected exception:")
        print(e)
        sys.exit(1)
