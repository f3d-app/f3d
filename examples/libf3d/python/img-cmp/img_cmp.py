import sys
from argparse import ArgumentParser

import f3d


def main(argv: list[str] | None = None):
    argparser = ArgumentParser()
    argparser.add_argument("image1")
    argparser.add_argument("image2")

    args = argparser.parse_args(argv)

    img1 = f3d.Image(args.image1)
    img2 = f3d.Image(args.image2)

    error = img1.compare(img2)
    if error > 0.05:
        print(f"Images are different ({error=})")
        return 1

    print(f"Images are identical ({error=})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
