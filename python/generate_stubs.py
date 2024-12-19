import re
import subprocess
import sys
from argparse import ArgumentParser
from contextlib import contextmanager
from difflib import unified_diff
from pathlib import Path
from tempfile import gettempdir
from typing import Iterable


def main():
    argparser = ArgumentParser()
    argparser.add_argument(
        "-o",
        "--into",
        help="output directory for the post-processed stubs (default: %(default)s)",
        default=f"{gettempdir()}/stubs",
    )
    args = argparser.parse_args()

    stubs = run_pybind11_stubgen(Path(args.into))
    if diff := postprocess_generated_stubs(stubs):
        print("\n".join(diff))


def run_pybind11_stubgen(out_dir: Path, module: str = "f3d"):
    stubgen_cmd = (
        # use current python interpreter to run stubs generation for the `f3d` module
        *(sys.executable, "-m", "pybind11_stubgen", module),
        # fix enum for default values in `Image.save()` and `Image.save_buffer()`
        *("--enum-class-locations", "SaveFormat:Image"),
        # ignore `f3d.vector3_t` and `f3d.point3_t` as we dont actually map them
        # but let them auto-convert from and to `tuple[float, float, float]`
        # (all occurrences will be postprocessed later)
        *("--ignore-unresolved-names", r"f3d\.(vector3_t|point3_t)"),
        # output directory so we can retrieve and post process
        *("--output-dir", out_dir),
    )
    with retrieve_changed_files(out_dir, f"{module}/**/*.pyi") as changed_files:
        subprocess.check_call(stubgen_cmd)
    return changed_files


def postprocess_generated_stubs(filenames: Iterable[Path]):
    replacements = [
        (
            # change `point3_t` and `vector3_t` parameter annotations and return types
            # to `tuple[float, float, float]`
            r"((:|->)\s*)f3d\.(vector3_t|point3_t)",
            r"\1tuple[float, float, float]",
        ),
        (
            # add missing template parameter to raw `os.PathLike` (`os.PathLike[str]`)
            r"(PathLike)(?!\[)",
            r"\1[str]",
        ),
        (
            # remove `_pybind11_conduit_v1_` static methods
            r"^\s+@staticmethod\s+def _pybind11_conduit_v1_\(\*args, *\*\*kwargs\):\s*\.\.\.[\n\r]",
            "",
        ),
    ]

    diff: list[str] = []

    for filename in filenames:
        processed = original = filename.read_text()
        for pattern, repl in replacements:
            processed = re.sub(pattern, repl, processed, flags=re.MULTILINE)
        filename.write_text(processed)

        diff += unified_diff(
            original.splitlines(),
            processed.splitlines(),
            str(filename),
            str(filename),
            n=1,
            lineterm="",
        )

    return diff


@contextmanager
def retrieve_changed_files(directory: Path, files_glob: str):
    mtimes = {f: f.stat().st_mtime for f in directory.glob(files_glob)}
    changed_files: list[Path] = []
    yield changed_files
    changed_files += (
        f for f in directory.glob(files_glob) if f.stat().st_mtime > mtimes.get(f, 0)
    )


if __name__ == "__main__":
    main()
