from pathlib import Path

from render_image import main


def test_render_image(tmp_path):
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"
    into = tmp_path
    main([str(file), "--into", str(into)])
