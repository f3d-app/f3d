import subprocess
import sys
from pathlib import Path


def test_minimal_qml():
    example_script = Path(__file__).parent.parent / "qml" / "minimal_qml.py"
    file = Path(__file__).parent.parent / "tests" / "data" / "cow.vtp"

    subprocess.run(
        [
            sys.executable,
            str(example_script),
            str(file),
            "--timeout",
            "1",
        ],
        capture_output=True,
        text=True,
    )
