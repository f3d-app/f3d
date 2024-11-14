import os

import pytest

import f3d


def callback_fn(args):
    print(args)
    return True


def test_command(capfd):
    engine = f3d.Engine.create(True)
    inter = engine.interactor

    # Check default commands can be removed
    actions = inter.get_command_callback_actions()
    for action in actions:
      inter.remove_command_callback(action)
    assert(len(inter.get_command_callback_actions()) == 0)

    # Check a command can be triggered
    inter.add_command_callback("my_cmd", callback_fn)
    inter.trigger_command("my_cmd arg1 arg2")
    inter.remove_command_callback("my_cmd")
    out, err = capfd.readouterr()
    assert out == "['arg1', 'arg2']\n"

    # Smoke test
    inter.create_default_command_callbacks()


def test_interaction_command():
    engine = f3d.Engine.create(True)
    inter = engine.interactor

    # Check default interactions can be removed
    binds = inter.get_interaction_binds()
    for [interaction, modifiers] in binds:
      inter.remove_interaction_command(interaction, modifiers)
    assert(len(inter.get_interaction_binds()) == 0)

    # Smoke test
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
    inter.create_default_interactions_commands()
