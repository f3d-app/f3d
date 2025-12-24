from pathlib import Path

import f3d

from offscreen_thumbnail import main


def test_offscreen_thumbnail(tmp_path: Path):
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"
    img = tmp_path / "thumbnail.png"
    assert main([str(file), str(img), "256"]) == 0
    assert img.exists()
    thumb = f3d.Image(img)
    assert (thumb.width, thumb.height) == (256, 256)
