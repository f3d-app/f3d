import sys
import f3d

if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(1)

    img_0 = f3d.Image(sys.argv[1])
    img_1 = f3d.Image(sys.argv[2])

    diff = f3d.Image()
    error = 0.0

    result = img_0.compare(img_1, 50, diff, error)

    sys.exit(0 if result else 1)
