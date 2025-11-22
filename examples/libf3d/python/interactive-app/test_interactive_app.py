from pathlib import Path

from interactive_app import main


def test_interactive_app():
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"
    main([str(file), "--timeout", "1"])
