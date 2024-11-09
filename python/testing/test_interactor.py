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
    bind = f3d.Interactor.InteractionBind
    bind.Interaction = "P"
    bind.Modifiers = f3d.Interactor.ModifierKeys.ANY
    inter.add_interaction_command(bind, ["dummy command"])
    bind.Modifiers = f3d.Interactor.ModifierKeys.NONE
    inter.add_interaction_command(bind, ["dummy command"])
    bind.Modifiers = f3d.Interactor.ModifierKeys.CTRL
    inter.add_interaction_command(bind, ["dummy command"])
    bind.Modifiers = f3d.Interactor.ModifierKeys.SHIFT
    inter.add_interaction_command(bind, ["dummy command"])
    bind.Modifiers = f3d.Interactor.ModifierKeys.CTRL_SHIFT
    inter.add_interaction_command(bind, ["dummy command"])
    inter.remove_interaction_command(bind)
