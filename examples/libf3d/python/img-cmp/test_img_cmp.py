from pathlib import Path

from img_cmp import main


def test_img_cmp():
    img = Path(__file__).parent.parent / "tests" / "baselines" / "cow.png"
    assert main([str(img), str(img)]) == 0
