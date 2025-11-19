import re
from argparse import ArgumentParser

import f3d


def main():
    parser = ArgumentParser()
    parser.add_argument("file")
    parser.add_argument(
        "--timeout",
        type=int,
        default=None,
        help="Optional timeout (in seconds) before closing the viewer.",
    )

    args = parser.parse_args()

    f3d.Engine.autoload_plugins()

    # Create a native-window engine
    eng = f3d.Engine.create(False)

    # Modify options using the struct-like API
    opt = eng.options
    opt.update(
        {
            "render.grid.enable": True,
            "render.show_edges": True,
            # UI overlays: axis + some HUD
            "ui.axis": True,
            "ui.fps": True,
            "ui.animation_progress": True,
            "ui.filename": True,
            # FXAA + tone mapping
            "render.effect.antialiasing.enable": True,
            "render.effect.antialiasing.mode": "fxaa",
            "render.effect.tone_mapping": True,
        }
    )

    AddCustomCommands(eng)
    AddCustomBindings(eng)

    # Add a model to the scene
    try:
        eng.scene.add(args.file)
    except Exception as e:
        print(e)

    # Initial render
    win = eng.window
    win.render()

    if args.timeout:

        def stop(eng):
            eng.interactor.stop()

        eng.interactor.start(args.timeout, lambda: stop(eng))
    else:
        eng.interactor.start()


def parse_size(size: str):
    if m := re.fullmatch(r"(\d+)([,x](\d+))?", size):
        w = int(m.group(1))
        h = int(m.group(3)) if m.group(3) else w
        return w, h
    else:
        raise ValueError(f"cannot parse size from {size!r}")


def AddCustomCommands(eng: f3d.Engine):
    # Get the interactor
    inter = eng.interactor
    inter.init_commands()

    # Reset options to initial state
    opt = eng.options

    # Keep a copy of the initial options
    initial_options = dict(opt)

    def reset_options(_):
        opt.update(initial_options)

    inter.add_command(
        "reset_options",
        reset_options,
        ("reset_options", "Reset basic render/UI options"),
    )

    # Increase animation speed factor
    def increase_animation_speed_factor(_):
        val = opt["scene.animation.speed_factor"]
        opt["scene.animation.speed_factor"] = val + 0.05

    inter.add_command(
        "increase_animation_speed_factor",
        increase_animation_speed_factor,
        ("increase_animation_speed_factor", "Increase animation speed factor"),
    )

    # Decrease animation speed factor
    def decrease_animation_speed_factor(_):
        val = opt["scene.animation.speed_factor"]
        opt["scene.animation.speed_factor"] = val - 0.05

    inter.add_command(
        "decrease_animation_speed_factor",
        decrease_animation_speed_factor,
        ("decrease_animation_speed_factor", "Decrease animation speed factor"),
    )

    # Toggle grid visibility
    inter.remove_command("toggle_grid")

    def toggle_grid(_):
        opt["render.grid.enable"] = not opt["render.grid.enable"]

    inter.add_command(
        "toggle_grid",
        toggle_grid,
        ("toggle_grid", "Toggle ground grid visibility"),
    )

    # Toggle axis
    inter.remove_command("toggle_axis")

    def toggle_axis(_):
        opt["ui.axis"] = not opt["ui.axis"]

    inter.add_command(
        "toggle_axis",
        toggle_axis,
        ("toggle_axis", "Toggle axis"),
    )

    # Toggle FXAA anti-aliasing
    inter.remove_command("toggle_fxaa")

    def toggle_fxaa(_):
        opt["render.effect.antialiasing.enable"] = not opt[
            "render.effect.antialiasing.enable"
        ]
        # keep mode at fxaa in this example
        opt["render.effect.antialiasing.mode"] = "fxaa"

    inter.add_command(
        "toggle_fxaa",
        toggle_fxaa,
        ("toggle_fxaa", "Toggle FXAA anti-aliasing"),
    )


def AddCustomBindings(eng: f3d.Engine):
    # Get the interactor
    inter = eng.interactor
    inter.init_bindings()

    opt = eng.options

    # Helpers for documentation
    def docTgl(doc, key):
        return doc, "ON" if opt[key] else "OFF"

    def docStr(doc):
        return doc, ""

    def docDblOpt(doc, key):
        val = opt[key]
        if val is None:
            return doc, "Unset"
        try:
            return doc, f"{float(val):.2f}"
        except:
            return doc, str(val)

    # R: reset options
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "R"),
        "reset_options",
        "Example",
        lambda: docStr("Reset Options"),
        f3d.Interactor.BindingType.OTHER,
    )

    # SHIFT + S: Increase animation speed
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.SHIFT, "S"),
        "increase_animation_speed_factor",
        "Example",
        lambda: docDblOpt("Increase animation speed", "scene.animation.speed_factor"),
        f3d.Interactor.BindingType.NUMERICAL,
    )

    # SHIFT + S: Increase animation speed
    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.CTRL, "S"),
        "decrease_animation_speed_factor",
        "Example",
        lambda: docDblOpt("Decrease animation speed", "scene.animation.speed_factor"),
        f3d.Interactor.BindingType.NUMERICAL,
    )

    # G: toggle grid
    inter.remove_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "G"),
    )

    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "G"),
        "toggle_grid",
        "Example",
        lambda: docTgl("Toggle grid", "render.grid.enable"),
        f3d.Interactor.BindingType.TOGGLE,
    )

    # X: toggle axis
    inter.remove_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "X"),
    )

    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "X"),
        "toggle_axis",
        "Example",
        lambda: docTgl("Toggle axis", "ui.axis"),
        f3d.Interactor.BindingType.TOGGLE,
    )

    # F: toggle FXAA
    inter.remove_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "F"),
    )

    inter.add_binding(
        f3d.InteractionBind(f3d.InteractionBind.ModifierKeys.NONE, "F"),
        "toggle_fxaa",
        "Example",
        lambda: docTgl("Toggle FXAA", "render.effect.antialiasing.enable"),
        f3d.Interactor.BindingType.TOGGLE,
    )


if __name__ == "__main__":
    main()
