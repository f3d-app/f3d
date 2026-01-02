from pathlib import Path

from minimal_qml import main

from PySide6.QtWidgets import QApplication


def test_minimal_qml():
    example_script = Path(__file__).parent.parent / "qml" / "minimal_qml.py"
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"

    if app := QApplication.instance():
        app.shutdown()
    main([str(example_script), str(file), "--timeout", "1"])
