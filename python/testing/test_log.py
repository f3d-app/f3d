import subprocess
import sys

from f3d import Log


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


def test_get_verbose_level():
    assert (
        run_python(
            "from f3d import Log",
            "Log.set_verbose_level(Log.DEBUG)",
            "print(Log.get_verbose_level() == Log.DEBUG)",
            "Log.set_verbose_level(Log.WARN)",
            "print(Log.get_verbose_level() == Log.WARN)",
        )
        == "True\nTrue\n"
    )


def test_forward():
    forwarded: dict[Log.VerboseLevel, list[str]] = {}

    def forward(level: Log.VerboseLevel, message: str):
        forwarded.setdefault(level, []).append(message)

    Log.forward(forward)
    Log.set_verbose_level(Log.VerboseLevel.DEBUG)

    Log.print(Log.VerboseLevel.DEBUG, "._.")
    Log.print(Log.VerboseLevel.INFO, "^_^")
    Log.print(Log.VerboseLevel.WARN, "O_o")
    Log.print(Log.VerboseLevel.ERROR, "x_x")

    assert forwarded == {
        Log.VerboseLevel.DEBUG: ["._."],
        Log.VerboseLevel.INFO: ["^_^"],
        Log.VerboseLevel.WARN: ["O_o"],
        Log.VerboseLevel.ERROR: ["x_x"],
    }


def run_python(*statements: str):
    return subprocess.check_output(
        [sys.executable, "-c", "; ".join(statements)],
        text=True,
    )
