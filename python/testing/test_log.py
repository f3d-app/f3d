from f3d import Log
from pytest import CaptureFixture, mark


@mark.parametrize(
    "level, expected_out, expected_err",
    [
        (None, "info\n", "warn\nerror\n"),
        (Log.DEBUG, "debug\ninfo\n", "warn\nerror\n"),
        (Log.INFO, "info\n", "warn\nerror\n"),
        (Log.WARN, "", "warn\nerror\n"),
        (Log.ERROR, "", "error\n"),
        (Log.QUIET, "", ""),
    ],
)
def test_levels(
    level: Log.VerboseLevel | None,
    expected_out: str,
    expected_err: str,
    capfd: CaptureFixture[str],
):
    if level is not None:
        Log.set_verbose_level(level)
    Log.set_use_coloring(False)

    Log.print(Log.DEBUG, "debug")
    Log.print(Log.INFO, "info")
    Log.print(Log.WARN, "warn")
    Log.print(Log.ERROR, "error")

    out, err = capfd.readouterr()
    assert out == expected_out
    assert err == expected_err


def test_coloring(capfd: CaptureFixture[str]):
    Log.set_verbose_level(Log.DEBUG)
    Log.set_use_coloring(True)

    Log.print(Log.DEBUG, "debug")
    Log.print(Log.INFO, "info")
    Log.print(Log.WARN, "warn")
    Log.print(Log.ERROR, "error")

    out, err = capfd.readouterr()
    assert out == "debug\ninfo\n"
    assert err == "\x1b[33mwarn\x1b[0m\n\x1b[31;1merror\x1b[0m\n"


def test_get_verbose_level():
    Log.set_verbose_level(Log.DEBUG)
    assert Log.get_verbose_level() == Log.DEBUG
    Log.set_verbose_level(Log.WARN)
    assert Log.get_verbose_level() == Log.WARN


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
