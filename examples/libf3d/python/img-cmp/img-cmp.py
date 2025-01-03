from argparse import ArgumentParser

import f3d

if __name__ == "__main__":
    argparser = ArgumentParser()
    argparser.add_argument("image1")
    argparser.add_argument("image2")

    args = argparser.parse_args()

    img1 = f3d.Image(args.image1)
    img2 = f3d.Image(args.image2)

    result, error = img1.compare(img2, 0.05)
    if result:
        print(f"Images are identical ({error=})")
    else:
        print(f"Images are different ({error=})")
