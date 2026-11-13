import pytest


def test_minimal_tkinter():
    pytest.importorskip("tkinter")  # tkinter is missing on linux for now
    pytest.importorskip("pyopengltk")
    from minimal_tkinter import main

    main(["--timeout", "1"])
