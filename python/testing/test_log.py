import subprocess
import sys


def test_default_level():
    assert (
        run_python(
            "from f3d import Log",
            "Log.set_use_coloring(False)",
            "Log.print(Log.DEBUG, 'debug')",
            "Log.print(Log.INFO, 'info')",
        )
        == "info\n"
    )


def test_debug():
    assert (
        run_python(
            "from f3d import Log",
            "Log.set_use_coloring(False)",
            "Log.set_verbose_level(Log.DEBUG)",
            "Log.print(Log.DEBUG, 'debug')",
        )
        == "debug\n"
    )


def test_coloring():
    assert (
        run_python(
            "from f3d import Log",
            "Log.set_use_coloring(True)",
            "Log.print(Log.INFO, 'info')",
        )
        == "info\x1b[0m\n"
    )


def run_python(*statements: str):
    return subprocess.check_output(
        [sys.executable, "-c", "; ".join(statements)],
        text=True,
    )
