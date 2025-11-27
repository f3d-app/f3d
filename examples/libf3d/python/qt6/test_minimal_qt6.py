import sys
from pathlib import Path

from minimal_qt6 import main


def test_minimal_qt6():
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"
    main([sys.argv[0], str(file), "--timeout", "1"])
