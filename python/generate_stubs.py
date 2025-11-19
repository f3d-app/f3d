import ast
import re
import subprocess
import sys
from argparse import ArgumentParser
from contextlib import contextmanager
from dataclasses import dataclass
from difflib import unified_diff
from pathlib import Path
from tempfile import gettempdir
from typing import Callable, Iterable, Literal, Sequence


def main(argv: Sequence[str] | None = None):
    argparser = ArgumentParser()
    argparser.add_argument(
        "-o",
        "--into",
        help="output directory for the post-processed stubs (default: %(default)s)",
        default=f"{gettempdir()}/stubs",
    )
    args = argparser.parse_args(argv)

    stubs = run_pybind11_stubgen(Path(args.into))
    if diff := postprocess_generated_stubs(stubs):
        print("\n".join(diff))


def run_pybind11_stubgen(
    out_dir: Path, module: str = "f3d", submodule: str = "f3d.pyf3d"
):
    stubgen_cmd = (
        # use current python interpreter to run stubs generation for the `f3d` module
        *(sys.executable, "-m", "pybind11_stubgen", submodule),
        # fix enum for default values in `Image.save()` and `Image.save_buffer()`
        *("--enum-class-locations", "SaveFormat:Image"),
        # more enums
        *("--enum-class-locations", "BindingType:Interactor"),
        *("--enum-class-locations", "LightType:f3d"),
        # ignore `f3d.vector3_t` and `f3d.point3_t` as we dont actually map them
        # but let them auto-convert from and to `tuple[float, float, float]`
        # (all occurrences will be postprocessed later)
        *("--ignore-unresolved-names", r"f3d\.(vector3_t|point3_t)"),
        # output directory so we can retrieve and post process
        *("--output-dir", out_dir),
        "--exit-code",
    )
    with retrieve_changed_files(out_dir, f"{module}/**/*.pyi") as changed_files:
        subprocess.check_call(stubgen_cmd)
    return changed_files


def postprocess_generated_stub(source: str):
    type_fixes = (
        TypeFix(
            # change `point3_t` and `vector3_t` to `tuple[float, float, float]`
            r"f3d\.(vector3_t|point3_t)",
            r"tuple[float, float, float]",
        ),
        TypeFix(
            # add missing template parameter to raw `os.PathLike` (`os.PathLike[str]`)
            r"(os\.PathLike)(?!\[)",
            r"\1[str]",
        ),
        TypeFix(
            # replace `Sequence[...]` by `list[...]` in function returns
            # because pybind outputs `std::vector` as `list`
            r"collections\.abc\.Sequence\[",
            r"list[",
            only_for=("return", "cb_arg"),
        ),
    )
    extra_imports = "pathlib", "os"

    transformer = StubTransformer(type_fixes, extra_imports)
    tree = ast.parse(source)
    tree = transformer.visit(tree)
    return ast.unparse(tree)


def postprocess_generated_stubs(filenames: Iterable[Path]):
    diff: list[str] = []

    for filename in filenames:
        source = filename.read_text()
        processed = postprocess_generated_stub(source)
        filename.write_text(processed)

        # parse and uparse source to normalize it and get a proper diff
        original = ast.unparse(ast.parse(source))
        diff += unified_diff(
            original.splitlines(),
            processed.splitlines(),
            str(filename),
            str(filename),
            n=1,
            lineterm="",
        )

    return diff


TypeFixTarget = Literal["arg", "return", "attr", "cb_arg", "cb_return"]


@dataclass
class TypeFix:
    pattern: str | re.Pattern[str]
    substitution: str | Callable[[re.Match[str]], str]
    only_for: tuple[TypeFixTarget, ...] | None = None


class StubTransformer(ast.NodeTransformer):
    def __init__(
        self, type_fixes: Iterable[TypeFix] = (), extra_imports: Iterable[str] = ()
    ) -> None:
        super().__init__()
        self.type_fixes = list(type_fixes)
        self.extra_imports = list(extra_imports)

    def visit_Import(self, node: ast.Import):
        """add extra imports before the first original import"""
        if self.extra_imports:
            new_imports = [ast.Import([ast.alias(name)]) for name in self.extra_imports]
            self.extra_imports.clear()
            return new_imports + [node]
        else:
            return node

    def _fix_annotation(self, annotation: ast.expr, target: TypeFixTarget):
        src = ast.unparse(annotation)
        for fix in self.type_fixes:
            if fix.only_for and target not in fix.only_for:
                continue
            if re.search(fix.pattern, src):
                annotation = ast.parse(
                    re.sub(fix.pattern, fix.substitution, src), "<string>", "eval"
                ).body
                src = ast.unparse(annotation)
        return annotation

    def visit_FunctionDef(self, node: ast.FunctionDef):
        if node.returns:
            node.returns = self._fix_annotation(node.returns, "return")
        return self.generic_visit(node)

    def visit_AnnAssign(self, node: ast.AnnAssign):
        if node.annotation:
            node.annotation = self._fix_annotation(node.annotation, "attr")
        return node

    def visit_arg(self, node: ast.arg):
        if isinstance(node.annotation, ast.Subscript) and re.match(
            r"(typing\.|collections\.abc\.)?Callable",
            ast.unparse(node.annotation.value),
        ):
            assert isinstance(node.annotation.slice, ast.Tuple)
            args, ret = node.annotation.slice.elts
            node.annotation.slice.elts = [
                self._fix_annotation(args, "cb_arg"),
                self._fix_annotation(ret, "cb_return"),
            ]
            return node
        elif node.annotation:
            node.annotation = self._fix_annotation(node.annotation, "arg")
        return node


@contextmanager
def retrieve_changed_files(directory: Path, files_glob: str):
    mtimes = {f: f.stat().st_mtime for f in directory.glob(files_glob)}
    changed_files: list[Path] = []
    yield changed_files
    changed_files += (
        f for f in directory.glob(files_glob) if f.stat().st_mtime > mtimes.get(f, 0)
    )


if __name__ == "__main__":  # pragma: nocover
    main()
