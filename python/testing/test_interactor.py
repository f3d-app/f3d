import pytest

import f3d


def print_fn(args: list[str]):
    print(args)


def doc_fn():
    return "dummyDoc", "dummyValue"


def compl_fn(args: list[str]):
    return ["compl"]


def test_command(capfd: pytest.CaptureFixture[str]):
    engine = f3d.Engine.create(True)
    inter = engine.interactor

    # Check default commands can be removed
    actions = inter.get_command_actions()
    for action in actions:
        inter.remove_command(action)
    assert len(inter.get_command_actions()) == 0

    # Check a command can be triggered
    inter.add_command("my_cmd", print_fn)
    inter.trigger_command("my_cmd arg1 arg2")
    inter.remove_command("my_cmd")
    out, _err = capfd.readouterr()
    assert out == "['arg1', 'arg2']\n"

    # Check a command can be triggered with #
    inter.add_command("my_cmd", print_fn)
    inter.trigger_command("my_cmd arg1 arg2 #000", False)
    inter.remove_command("my_cmd")
    out, _err = capfd.readouterr()
    assert out == "['arg1', 'arg2', '#000']\n"

    # Smoke test
    inter.init_commands()
    inter.add_command("my_cmd2", print_fn, ("my_cmd2", "doc"), compl_fn)


def test_command_completion(capfd: pytest.CaptureFixture[str]):
    engine = f3d.Engine.create(True)
    inter = engine.interactor

    # Check default commands can be removed
    actions = inter.get_command_actions()
    for action in actions:
        inter.remove_command(action)
    assert len(inter.get_command_actions()) == 0

    # Smoke test
    inter.init_commands()
    inter.add_command("cmd", print_fn, ("cmd", "doc"), compl_fn)

    engine.window.render()

    inter.trigger_keyboard_key(inter.InputAction.PRESS, "Escape")
    inter.trigger_keyboard_key(inter.InputAction.RELEASE, "Escape")
    for c in "cmd ":
        inter.trigger_text_character(ord(c))
    inter.trigger_keyboard_key(inter.InputAction.PRESS, "Tab")
    inter.trigger_keyboard_key(inter.InputAction.RELEASE, "Tab")


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
    assert len(inter.get_binds()) == 0

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
        f3d.Interactor.BindingType.CYCLIC,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL, "P"),
        "dummy command",
        "DummyGroup",
        doc_fn,
        f3d.Interactor.BindingType.NUMERICAL,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.SHIFT, "P"),
        "dummy command",
        "DummyGroup",
        doc_fn,
        f3d.Interactor.BindingType.TOGGLE,
    )
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL_SHIFT, "P"),
        ["dummy command", "dummy command"],
        "DummyGroup",
        doc_fn,
        f3d.Interactor.BindingType.OTHER,
    )
    assert len(inter.get_bind_groups()) == 1
    assert len(inter.get_binds()) == 5
    assert (
        inter.get_binding_documentation(
            f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL, "P")
        )
        == doc_fn()
    )
    assert (
        inter.get_binding_type(
            f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL, "P")
        )
        == f3d.Interactor.BindingType.NUMERICAL
    )

    inter.init_bindings()


def test_trigger_key(capfd: pytest.CaptureFixture[str]):
    engine = f3d.Engine.create(True)

    engine.window.render()

    engine.interactor.trigger_keyboard_key(f3d.Interactor.InputAction.PRESS, "E")
    engine.interactor.trigger_keyboard_key(f3d.Interactor.InputAction.RELEASE, "E")
    assert engine.options["render.show_edges"] is True

    engine.interactor.trigger_mod_update(f3d.Interactor.InputModifier.SHIFT)
    engine.interactor.trigger_keyboard_key(f3d.Interactor.InputAction.PRESS, "L")
    engine.interactor.trigger_keyboard_key(f3d.Interactor.InputAction.RELEASE, "L")
    assert engine.options["render.light.intensity"] == 0.98

    engine.interactor.trigger_mouse_button(
        f3d.Interactor.InputAction.PRESS, f3d.Interactor.MouseButton.LEFT
    )
    engine.interactor.trigger_mouse_position(100, 100)
    engine.interactor.trigger_mouse_wheel(f3d.Interactor.WheelDirection.FORWARD)
    engine.interactor.trigger_text_character(0)
