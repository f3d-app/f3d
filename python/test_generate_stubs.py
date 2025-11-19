import ast
from pathlib import Path
from tempfile import TemporaryDirectory
from textwrap import dedent

from generate_stubs import StubTransformer, TypeFix, main, postprocess_generated_stub
from pytest import importorskip


def test_main():
    importorskip("f3d")
    importorskip("pybind11_stubgen")
    with TemporaryDirectory() as tmp:
        main(["--into", tmp])
        assert (Path(tmp) / "f3d/pyf3d.pyi").is_file()


def test_transformer_f3d_point_vector():
    src = """
    class A:
        p: f3d.point3_t
        v: f3d.vector3_t
        def f(p: f3d.point3_t) -> f3d.point3_t: ...
        def g(v: f3d.vector3_t) -> f3d.vector3_t: ...
    """
    expected = """
    class A:
        p: tuple[float,float,float]
        v: tuple[float,float,float]
        def f(p: tuple[float,float,float]) -> tuple[float,float,float]: ...
        def g(v: tuple[float,float,float]) -> tuple[float,float,float]: ...
    """
    assert postprocess_generated_stub(dedent(src)) == parse_unparse(expected)


def test_transformer_f3d_lists():
    src = """
    class A:
        xs: list[T]  # should stay
        def f(xs: collections.abc.Sequence[T]) -> collections.abc.Sequence[T]: ...  # return should be `list`
        def g(cb: Callable[[collections.abc.Sequence[T]], collections.abc.Sequence[T]]): ...  # cb arg shloud become `list`
    """
    expected = """
    class A:
        xs: list[T]
        def f(xs: collections.abc.Sequence[T]) -> list[T]: ...
        def g(cb: Callable[[list[T]], collections.abc.Sequence[T]]): ...
    """
    assert postprocess_generated_stub(dedent(src)) == parse_unparse(expected)


def test_transformer_f3d_path():
    src = """
    class A:
        xs: pathlib.Path  # should be `pathlib.Path`
        def f(p: os.PathLike | str | bytes) -> pathlib.Path: ...   # arg should be `os.PathLike[str]|str|bytes`
    """
    expected = """
    class A:
        xs: pathlib.Path
        def f(p: os.PathLike[str] | str | bytes) -> pathlib.Path: ...
    """
    assert postprocess_generated_stub(dedent(src)) == parse_unparse(expected)


def test_transformer_typefix_matching():
    transformer = StubTransformer(
        type_fixes=[
            TypeFix(r"T", t.upper(), only_for=(t,))
            for t in ("arg", "return", "attr", "cb_arg", "cb_return")
        ],
    )
    src = """
    __all__: list[T] = ...

    def f1(a: T, b: T | None) -> list[T]:...

    class A:
        a: T
        b: T
        def f(a: T, b: T) -> T | None: ...
        def f(xs: list[T], cb: Callable[[T, T], T]) -> T: ...
    """
    expected = """
    __all__: list[ATTR] = ...

    def f1(a: ARG, b: ARG | None) -> list[RETURN]: ...

    class A:
        a: ATTR
        b: ATTR
        def f(a: ARG, b: ARG) -> RETURN | None: ...
        def f(xs: list[ARG], cb: Callable[[CB_ARG, CB_ARG], CB_RETURN]) -> RETURN: ...
    """
    tree = transformer.visit(ast.parse(dedent(src)))
    assert ast.unparse(tree) == parse_unparse(expected)


def test_transformer_extra_imports():
    transformer = StubTransformer(
        extra_imports=("hello", "world"),
    )
    src = """
    from __future__ import annotations
    # extra imports should go here
    import foo
    import bar
    """
    expected = """
    from __future__ import annotations
    import hello
    import world
    import foo
    import bar
    """
    tree = transformer.visit(ast.parse(dedent(src)))
    assert ast.unparse(tree) == parse_unparse(expected)


def parse_unparse(src: str):
    return ast.unparse(ast.parse(dedent(src)))
