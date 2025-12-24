from argparse import ArgumentParser
from pathlib import Path

import f3d

TEST_DATA_DIR = Path(__file__).parent.parent.parent.parent.parent / "testing/data"


def main(argv: list[str] | None = None):
    argparser = ArgumentParser()
    argparser.add_argument("models", nargs="*")
    argparser.add_argument(
        "--timeout",
        type=int,
        default=None,
        help="Optional timeout (in seconds) before closing the viewer.",
    )

    args = argparser.parse_args(argv)

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

    if args.timeout:
        # For testing purposes only, exit after `timeout` seconds
        eng.interactor.start(args.timeout, eng.interactor.stop)
    else:
        eng.interactor.start()


if __name__ == "__main__":
    main()
