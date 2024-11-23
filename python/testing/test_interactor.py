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
    groups = inter.get_bind_groups()
    for group in groups:
        binds = inter.get_binds_for_group(group)
        for bind in binds:
            inter.remove_binding(bind)
    assert len(inter.get_bind_groups()) == 0

    # Smoke test
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.ANY, "P"),
        "dummy command",
        "DummyGroup",
        doc_fn,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "P"),
        "dummy command",
        "DummyGroup",
        doc_fn,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL, "P"),
        "dummy command",
        "DummyGroup",
        doc_fn,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.SHIFT, "P"),
        "dummy command",
        "DummyGroup",
        doc_fn,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL_SHIFT, "P"),
        ["dummy command", "dummy command"],
        "DummyGroup",
        doc_fn,
    )
    inter.init_bindings()
