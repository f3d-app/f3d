from pathlib import Path

from minimal_qt6 import main

from PySide6.QtWidgets import QApplication


def test_minimal_qt6():
    example_script = Path(__file__).parent.parent / "qt6" / "minimal_qt6.py"
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"
    if app := QApplication.instance():
        app.shutdown()
    main(
        [str(example_script), str(file), "--timeout", "1"],
    )
