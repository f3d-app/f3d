import subprocess
import sys
from pathlib import Path

F3D_BIN_PATH = Path(__file__).parent / "bin" / "f3d"
if sys.platform == "win32":
    F3D_BIN_PATH = F3D_BIN_PATH.with_suffix(".exe")


def run(args):
    return subprocess.call([F3D_BIN_PATH] + args)


def main():
    return SystemExit(run(sys.argv[1:]))


if __name__ == "__main__":
    main()
