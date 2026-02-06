import sys
from argparse import ArgumentParser
from pathlib import Path
from collections import deque

import f3d

from PySide6.QtGui import QGuiApplication, QOpenGLContext
from PySide6.QtCore import (
    QTimer,
    QCoreApplication,
    Property,
    Slot,
    Qt,
)
from PySide6.QtQml import QQmlApplicationEngine, qmlRegisterType
from PySide6.QtQuick import QQuickFramebufferObject, QQuickItem


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv

    parser = ArgumentParser()
    parser.add_argument("file", help="3D model file to open")
    parser.add_argument(
        "--timeout",
        type=int,
        help="Optional timeout (in seconds) before closing the viewer.",
    )

    # Let argparse handle our options, and leave the rest to Qt
    args, qt_args = parser.parse_known_args(argv[1:])

    model_path = Path(args.file)
    if not model_path.exists():
        print(f"File not found: {model_path}")
        return 1

    app = QGuiApplication([argv[0]] + qt_args)

    qmlRegisterType(F3DView, "F3D", 1, 0, "F3DView")

    engine = QQmlApplicationEngine()
    # expose initial model path to QML
    engine.rootContext().setContextProperty("initialModelPath", str(model_path))

    qml_file = Path(__file__).parent / "Main.qml"
    engine.load(qml_file.as_uri())

    if not engine.rootObjects():
        print("Failed to load QML")
        return 1

    # For testing purposes only, shutdown the example after `timeout` seconds
    if args.timeout:
        QTimer.singleShot(args.timeout * 1000, QCoreApplication.quit)

    return app.exec()


class F3DView(QQuickFramebufferObject):
    """
    QML-exposed item that uses libf3d to render a 3D model
    inside Qt Quick via QQuickFramebufferObject, with event
    forwarding to libf3d::interactor.
    """

    def __init__(self, parent: QQuickItem | None = None):
        super().__init__(parent)
        self._model_path: str = ""
        self._renderer: F3DRenderer | None = None

        self.setMirrorVertically(True)
        # Request depth buffer in the texture format
        self.setTextureFollowsItemSize(True)

    @Property(str)
    def modelPath(self) -> str:
        return self._model_path

    @modelPath.setter
    def modelPath(self, path: str) -> None:
        self._model_path = path

    def createRenderer(self):
        self._renderer = F3DRenderer(self)
        return self._renderer

    def releaseResources(self):
        if self._renderer is not None:
            self._renderer = None

    # Mouse
    @Slot(float, float, int, int)
    def mousePress(self, x: float, y: float, button: int, modifiers: int) -> None:
        if self._renderer is not None:
            self._renderer.queue_mouse_press(x, y, button, modifiers)

    @Slot(float, float, int, int)
    def mouseMove(self, x: float, y: float, buttons: int, modifiers: int) -> None:
        if self._renderer is not None:
            self._renderer.queue_mouse_move(x, y, buttons, modifiers)

    @Slot(float, float, int, int)
    def mouseRelease(self, x: float, y: float, button: int, modifiers: int) -> None:
        if self._renderer is not None:
            self._renderer.queue_mouse_release(x, y, button, modifiers)

    # Wheel
    @Slot(int, int, int)
    def mouseWheel(self, delta_x: int, delta_y: int, modifiers: int) -> None:
        if self._renderer is not None:
            self._renderer.queue_wheel(delta_x, delta_y, modifiers)

    # Keyboard
    @Slot(int, str, int)
    def keyPress(self, key: int, text: str, modifiers: int) -> None:
        if self._renderer is not None:
            self._renderer.queue_key_press(key, text, modifiers)

    @Slot(int, int)
    def keyRelease(self, key: int, modifiers: int) -> None:
        if self._renderer is not None:
            self._renderer.queue_key_release(key, modifiers)


class F3DRenderer(QQuickFramebufferObject.Renderer):
    """
    Actual OpenGL renderer for F3DView.
    """

    def __init__(self, item: F3DView):
        super().__init__()
        self.item = item

        f3d.Engine.autoload_plugins()

        self.engine = f3d.Engine.create_external(
            QOpenGLContext.currentContext().getProcAddress
        )

        if self.item.modelPath:
            self.engine.scene.add(self.item.modelPath)

        win = self.engine.window
        win.size = (int(self.item.width()), int(self.item.height()))

        opt = self.engine.options
        opt.update(
            {
                "render.grid.enable": True,
                "render.show_edges": False,
                "ui.axis": True,
                "ui.fps": True,
                "ui.filename": True,
                "render.background.color": [0.15, 0.15, 0.15],
            }
        )

        self.interactor = self.engine.interactor

        self._events = deque()

    def queue_mouse_press(self, x: float, y: float, button: int, modifiers: int):
        self._events.append(("mouse_press", (x, y, button, modifiers)))
        self.update()

    def queue_mouse_move(self, x: float, y: float, buttons: int, modifiers: int):
        self._events.append(("mouse_move", (x, y, buttons, modifiers)))
        self.update()

    def queue_mouse_release(self, x: float, y: float, button: int, modifiers: int):
        self._events.append(("mouse_release", (x, y, button, modifiers)))
        self.update()

    def queue_wheel(self, dx: int, dy: int, modifiers: int):
        self._events.append(("wheel", (dx, dy, modifiers)))
        self.update()

    def queue_key_press(self, key: int, text: str, modifiers: int):
        self._events.append(("key_press", (key, text, modifiers)))
        self.update()

    def queue_key_release(self, key: int, modifiers: int):
        self._events.append(("key_release", (key, modifiers)))
        self.update()

    # --------------------------
    # Event mapping helpers
    # --------------------------

    def _update_modifiers(self, mods: int) -> None:
        inter = self.interactor
        m = Qt.KeyboardModifier(mods)

        if (m & Qt.KeyboardModifier.ControlModifier) and (
            m & Qt.KeyboardModifier.ShiftModifier
        ):
            inter.trigger_mod_update(inter.InputModifier.CTRL_SHIFT)
        elif m & Qt.KeyboardModifier.ControlModifier:
            inter.trigger_mod_update(inter.InputModifier.CTRL)
        elif m & Qt.KeyboardModifier.ShiftModifier:
            inter.trigger_mod_update(inter.InputModifier.SHIFT)
        else:
            inter.trigger_mod_update(inter.InputModifier.NONE)

    def _map_mouse_button(self, button: int):
        inter = self.interactor

        b = Qt.MouseButton(button)
        if b == Qt.MouseButton.LeftButton:
            return inter.MouseButton.LEFT
        if b == Qt.MouseButton.RightButton:
            return inter.MouseButton.RIGHT
        if b == Qt.MouseButton.MiddleButton:
            return inter.MouseButton.MIDDLE
        return None

    def _key_sym_from_key_and_text(self, key: int, text: str) -> str:
        # A–Z
        if Qt.Key.Key_A <= key <= Qt.Key.Key_Z:
            return chr(key).upper()

        special = {
            Qt.Key.Key_Backspace: "BackSpace",
            Qt.Key.Key_Escape: "Escape",
            Qt.Key.Key_Return: "Return",
            Qt.Key.Key_Enter: "Return",
            Qt.Key.Key_Space: "Space",
            Qt.Key.Key_Tab: "Tab",
        }
        if key in special:
            return special[key]

        if text:
            return text
        return ""

    # --------------------------
    # Main render function
    # --------------------------

    def render(self) -> None:
        win = self.engine.window
        win.size = (int(self.item.width()), int(self.item.height()))

        inter = self.interactor

        while self._events:
            kind, data = self._events.popleft()

            if kind == "mouse_press":
                x, y, button, mods = data
                self._update_modifiers(mods)
                mb = self._map_mouse_button(button)
                if mb is not None:
                    inter.trigger_mouse_position(x, y)
                    inter.trigger_mouse_button(inter.InputAction.PRESS, mb)

            elif kind == "mouse_move":
                x, y, buttons, mods = data
                self._update_modifiers(mods)
                inter.trigger_mouse_position(x, y)

            elif kind == "mouse_release":
                x, y, button, mods = data
                self._update_modifiers(mods)
                mb = self._map_mouse_button(button)
                if mb is not None:
                    inter.trigger_mouse_button(inter.InputAction.RELEASE, mb)
                    inter.trigger_mouse_position(x, y)

            elif kind == "wheel":
                dx, dy, mods = data
                self._update_modifiers(mods)

                if abs(dy) >= abs(dx):
                    direction = (
                        inter.WheelDirection.FORWARD
                        if dy > 0
                        else inter.WheelDirection.BACKWARD
                    )
                else:
                    direction = (
                        inter.WheelDirection.RIGHT
                        if dx > 0
                        else inter.WheelDirection.LEFT
                    )
                inter.trigger_mouse_wheel(direction)

            elif kind == "key_press":
                key, text, mods = data
                self._update_modifiers(mods)
                key_sym = self._key_sym_from_key_and_text(key, text)
                if key_sym:
                    inter.trigger_keyboard_key(inter.InputAction.PRESS, key_sym)
                if text:
                    for ch in text:
                        inter.trigger_text_character(ord(ch))

            elif kind == "key_release":
                key, mods = data
                self._update_modifiers(mods)
                key_sym = self._key_sym_from_key_and_text(key, "")
                if key_sym:
                    inter.trigger_keyboard_key(inter.InputAction.RELEASE, key_sym)

        inter.trigger_event_loop(1.0 / 30.0)

        win.render()

        self.update()


if __name__ == "__main__":
    raise SystemExit(main())
