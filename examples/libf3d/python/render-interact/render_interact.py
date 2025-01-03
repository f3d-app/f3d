from argparse import ArgumentParser
from pathlib import Path

import f3d

TEST_DATA_DIR = Path(__file__).parent.parent.parent.parent.parent / "testing/data"

if __name__ == "__main__":
    argparser = ArgumentParser()
    argparser.add_argument("models", nargs="*")

    args = argparser.parse_args()

    input_paths = args.models or [TEST_DATA_DIR / "suzanne.obj"]

    # Create a native window engine
    eng = f3d.Engine.create()

    # Add a model
    try:
        eng.scene.add(input_paths)
    except RuntimeError as e:
        print(e)

    # Render
    eng.window.render()

    eng.interactor.start()
