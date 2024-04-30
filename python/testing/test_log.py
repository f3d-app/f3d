import subprocess
import sys


def test_default():
    assert (
        run_python(
            "from f3d import Log",
            "Log.print(Log.DEBUG, 'debug')",
            "Log.print(Log.INFO, 'info')",
        )
        == "info\x1b[0m\n"
    )


def test_debug():
    assert (
        run_python(
            "from f3d import Log",
            "Log.set_verbose_level(Log.DEBUG)",
            "Log.print(Log.DEBUG, 'debug')",
        )
        == "debug\x1b[0m\n"
    )


def test_no_coloring():
    assert (
        run_python(
            "from f3d import Log",
            "Log.set_use_coloring(False)",
            "Log.print(Log.INFO, 'info')",
        )
        == "info\n"
    )


def run_python(*statements: str):
    return subprocess.check_output(
        [sys.executable, "-c", "; ".join(statements)],
        text=True,
    )
