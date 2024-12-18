import re
import subprocess
import sys
from argparse import ArgumentParser
from pathlib import Path
from tempfile import TemporaryDirectory, gettempdir


def main():
    argparser = ArgumentParser()
    argparser.add_argument(
        "-o",
        "--into",
        help="output directory for the post-processed stubs",
        default=gettempdir(),
    )
    args = argparser.parse_args()

    with TemporaryDirectory() as tmp_dir:
        run_pybind11_stubgen(tmp_dir)
        postprocess_generated_stubs(tmp_dir, args.into)


def run_pybind11_stubgen(out_dir: str):
    stubgen_cmd = (
        # use current python interpreter to run stubs generation for the `f3d` module
        *(sys.executable, "-m", "pybind11_stubgen", "f3d"),
        # fix enum for default values in `Image.save()` and `Image.save_buffer()`
        *("--enum-class-locations", "SaveFormat:Image"),
        # ignore `f3d.vector3_t` and `f3d.point3_t` as we dont actually map them
        # but let them auto-convert from and to `tuple[float, float, float]`
        # (all occurrences will be postprocessed later)
        *("--ignore-unresolved-names", r"f3d\.(vector3_t|point3_t)"),
        # output in temporary directory as we're going to post process
        *("--output-dir", out_dir),
    )
    subprocess.check_call(stubgen_cmd)


def postprocess_generated_stubs(stubs_dir: str, into: str):
    replacements = [
        (
            # change `point3_t` and `vector3_t` parameter annotations and return types
            # to `tuple[float, float, float]`
            r"((:|->)\s*)f3d\.(vector3_t|point3_t)",
            r"\1tuple[float, float, float]",
        ),
        (
            # remove `point3_t` and `vector3_t` being imported `as tuple`
            r"from builtins import tuple as (vector3_t|point3_t)[\n\r]+",
            r"",
        ),
        (
            # add missing template parameter to raw `os.PathLike` (`os.PathLike[str | bytes]`)
            r"(PathLike)(?!\[)",
            r"\1[str | bytes]",
        ),
        (
            # remove `_pybind11_conduit_v1_` static methods
            r"^\s+@staticmethod[\n\r]+\s+def _pybind11_conduit_v1_\(\*args, \*\*kwargs\):[\n\r]+\s+\.\.\.[\n\r]",
            "",
        ),
    ]

    tmp_dir = Path(stubs_dir)
    out_dir = Path(into)

    for tmp_fn in tmp_dir.glob("**/*.pyi"):
        rel_fn = tmp_fn.relative_to(tmp_dir)
        out_fn = out_dir / rel_fn
        out_fn.parent.mkdir(exist_ok=True, parents=True)

        src = tmp_fn.read_text()
        for pattern, repl in replacements:
            src = re.sub(pattern, repl, src, flags=re.MULTILINE)
        out_fn.write_text(src)

        try:
            subprocess.call(["diff", "-u", str(tmp_fn), str(out_fn)])
        except IOError:
            pass  # no `diff` executable


if __name__ == "__main__":
    main()
