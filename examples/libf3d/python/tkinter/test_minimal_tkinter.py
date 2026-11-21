import pytest


def test_minimal_tkinter():
    pytest.importorskip(
        "tkinter", exc_type=ImportError
    )  # tkinter is not importable on linux for now, see https://github.com/actions/setup-python/issues/1121
    pytest.importorskip("pyopengltk", exc_type=ImportError)
    from minimal_tkinter import main

    main(["--timeout", "1"])
