import os

import pytest

import f3d


def callback_fn(args):
    print(args)


def doc_fn():
    return ["dummyDoc", "dummyValue"]


def test_command(capfd):
    engine = f3d.Engine.create(True)
    inter = engine.interactor

    # Check default commands can be removed
    actions = inter.get_command_actions()
    for action in actions:
        inter.remove_command(action)
    assert len(inter.get_command_actions()) == 0

    # Check a command can be triggered
    inter.add_command("my_cmd", callback_fn)
    inter.trigger_command("my_cmd arg1 arg2")
    inter.remove_command("my_cmd")
    out, err = capfd.readouterr()
    assert out == "['arg1', 'arg2']\n"

    # Smoke test
    inter.init_commands()


def test_binding():
    engine = f3d.Engine.create(True)
    inter = engine.interactor

    # Check default interactions can be removed
    binds = inter.get_binding_interactions()
    for [interaction, modifiers] in binds:
        inter.remove_binding(interaction, modifiers)
    assert len(inter.get_binding_interactions()) == 0

    # Smoke test
    inter.add_binding("P", f3d.Interactor.ModifierKeys.ANY, "dummy command", doc_fn)
    inter.add_binding("P", f3d.Interactor.ModifierKeys.NONE, "dummy command", doc_fn)
    inter.add_binding("P", f3d.Interactor.ModifierKeys.CTRL, "dummy command", doc_fn)
    inter.add_binding("P", f3d.Interactor.ModifierKeys.SHIFT, "dummy command", doc_fn)
    inter.add_binding(
        "P",
        f3d.Interactor.ModifierKeys.CTRL_SHIFT,
        ["dummy command", "dummy command"],
        doc_fn,
    )
    inter.init_bindings()
