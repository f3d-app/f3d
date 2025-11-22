import pytest

pytest.importorskip("tkinter")
pytest.importorskip("pyopengltk")

from minimal_tkinter import main


def test_minimal_tkinter():
    main(["--timeout", "1"])
