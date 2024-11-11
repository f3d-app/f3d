import os

import pytest

import f3d


def callback_fn(args):
    print(args)
    return True


def test_command(capfd):
    engine = f3d.Engine.create(True)
    inter = engine.interactor
    inter.add_command_callback("my_cmd", callback_fn)
    inter.trigger_command("my_cmd arg1 arg2")
    inter.remove_command_callback("my_cmd")
    out, err = capfd.readouterr()
    assert out == "['arg1', 'arg2']\n"


def test_interaction_command():
    # Smoke test
    engine = f3d.Engine.create(True)
    inter = engine.interactor
    inter.add_interaction_command("P", f3d.Interactor.ModifierKeys.ANY, "dummy command")
    inter.add_interaction_command(
        "P", f3d.Interactor.ModifierKeys.NONE, "dummy command"
    )
    inter.add_interaction_command(
        "P", f3d.Interactor.ModifierKeys.CTRL, "dummy command"
    )
    inter.add_interaction_command(
        "P", f3d.Interactor.ModifierKeys.SHIFT, "dummy command"
    )
    inter.add_interaction_commands(
        "P", f3d.Interactor.ModifierKeys.CTRL_SHIFT, ["dummy command", "dummy command"]
    )
    inter.remove_interaction_command("P", f3d.Interactor.ModifierKeys.ANY)
