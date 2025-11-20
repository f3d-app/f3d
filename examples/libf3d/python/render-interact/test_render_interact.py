from pathlib import Path

from render_interact import main


def test_render_interact():
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"
    main([str(file), "--timeout", "1"])
