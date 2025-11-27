import sys
from argparse import ArgumentParser
from pathlib import Path

import f3d
from PySide6.QtGui import QKeyEvent, QMouseEvent, QWheelEvent
from PySide6.QtWidgets import QApplication, QMainWindow, QWidget
from PySide6.QtOpenGLWidgets import QOpenGLWidget
from PySide6.QtCore import Qt, QPointF, QTimer, QCoreApplication


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv

    parser = ArgumentParser()
    parser.add_argument("file")
    parser.add_argument(
        "--timeout",
        type=int,
        help="Optional timeout (in seconds) before closing the viewer.",
    )

    # Let argparse handle our options, and leave the rest to Qt
    args, qt_args = parser.parse_known_args(argv[1:])

    app = QApplication([argv[0]] + qt_args)

    model_path = Path(args.file)
    if not model_path.exists():
        print(f"File not found: {model_path}")
        return 1

    win = MainWindow(str(model_path))
    win.show()

    # For testing purposes only, shutdown the example after `timeout` seconds
    if args.timeout:
        QTimer.singleShot(args.timeout * 1000, QCoreApplication.quit)

    return app.exec()


class MainWindow(QMainWindow):
    def __init__(self, file_path: str, parent: QWidget | None = None):
        super().__init__(parent)
        self.setWindowTitle("libf3d + PySide6 example")

        self.viewer = F3DWidget(file_path, self)
        self.setCentralWidget(self.viewer)
        self.resize(800, 600)


class F3DWidget(QOpenGLWidget):
    """
    QOpenGLWidget that uses libf3d to render a 3D model
    inside Qt's OpenGL context.
    """

    def __init__(self, file_path: str, parent: QWidget | None = None):
        super().__init__(parent)
        self.file_path = file_path
        self.engine: f3d.Engine | None = None
        self.interactor: f3d.Interactor | None = None
        self.timer: QTimer | None = None

        # Allow this widget to receive key events
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)

    # ------------------------------------------------------------------
    # OpenGL / F3D setup
    # ------------------------------------------------------------------

    def initializeGL(self) -> None:
        """Called once when the OpenGL context is created."""
        f3d.Engine.autoload_plugins()

        self.engine = f3d.Engine.create_external(self.context().getProcAddress)

        # Retrieve the interactor for event forwarding
        self.interactor = self.engine.interactor

        self.timer = QTimer(self)
        self.timer.setInterval(30)
        self.timer.timeout.connect(self._on_tick)
        self.timer.start()

        self.engine.scene.add(self.file_path)

        win = self.engine.window
        win.size = (self.width(), self.height())

        opt = self.engine.options
        opt.update(
            {
                "render.grid.enable": True,
                "render.show_edges": False,
                "ui.axis": True,
                "ui.fps": True,
                "ui.filename": True,
            }
        )

    def _on_tick(self) -> None:
        if self.engine is None or self.interactor is None:
            return
        self.makeCurrent()
        self.interactor.trigger_event_loop(1.0 / 30.0)
        self.update()
        self.doneCurrent()

    def resizeGL(self, width: int, height: int) -> None:
        if self.engine is not None:
            self.engine.window.size = (max(1, width), max(1, height))

    def paintGL(self) -> None:
        if self.engine is not None:
            self.engine.window.render()

    # ------------------------------------------------------------------
    # Forwarding helpers
    # ------------------------------------------------------------------

    def _update_modifiers(self, mods: Qt.KeyboardModifier) -> None:
        """
        Translate Qt modifiers (Shift/Ctrl) into f3d::interactor::InputModifier
        and forward them.
        """
        if self.interactor is None:
            return

        inter = self.interactor

        if (mods & Qt.KeyboardModifier.ControlModifier) and (
            mods & Qt.KeyboardModifier.ShiftModifier
        ):
            inter.trigger_mod_update(inter.InputModifier.CTRL_SHIFT)
        elif mods & Qt.KeyboardModifier.ControlModifier:
            inter.trigger_mod_update(inter.InputModifier.CTRL)
        elif mods & Qt.KeyboardModifier.ShiftModifier:
            inter.trigger_mod_update(inter.InputModifier.SHIFT)
        else:
            inter.trigger_mod_update(inter.InputModifier.NONE)

    def _map_mouse_button(self, button: Qt.MouseButton):
        """Map a Qt mouse button to f3d::interactor::MouseButton."""
        if self.interactor is None:
            return None

        inter = self.interactor
        if button == Qt.MouseButton.LeftButton:
            return inter.MouseButton.LEFT
        if button == Qt.MouseButton.RightButton:
            return inter.MouseButton.RIGHT
        if button == Qt.MouseButton.MiddleButton:
            return inter.MouseButton.MIDDLE
        return None

    def _key_sym_from_event(self, event: QKeyEvent) -> str:
        """
        Convert a QKeyEvent into a keySym string expected by libf3d.
        Mainly covers common F3D bindings (G, X, etc.).
        """
        key: int = event.key()

        # A–Z -> "A"..."Z"
        if Qt.Key.Key_A <= key <= Qt.Key.Key_Z:
            return chr(key).upper()

        # Special keys
        special = {
            Qt.Key.Key_Backspace: "BackSpace",
            Qt.Key.Key_Escape: "Escape",
            Qt.Key.Key_Return: "Return",
            Qt.Key.Key_Enter: "Return",
            Qt.Key.Key_Space: "Space",
            Qt.Key.Key_Tab: "Tab",
        }
        try:
            return special[key]
        except KeyError:
            return event.text()

    # ------------------------------------------------------------------
    # Mouse interaction forwarding
    # ------------------------------------------------------------------

    def mousePressEvent(self, event: QMouseEvent) -> None:
        if self.interactor is None:
            return None
        self.makeCurrent()

        self._update_modifiers(event.modifiers())
        inter = self.interactor
        button = self._map_mouse_button(event.button())

        if button is not None:
            pos: QPointF = event.position()
            inter.trigger_mouse_position(pos.x(), pos.y())
            inter.trigger_mouse_button(inter.InputAction.PRESS, button)
            self.update()

        self.doneCurrent()
        return None

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        if self.interactor is None:
            return None

        self.makeCurrent()

        self._update_modifiers(event.modifiers())
        inter = self.interactor
        pos: QPointF = event.position()
        inter.trigger_mouse_position(pos.x(), pos.y())
        self.update()

        self.doneCurrent()
        return None

    def mouseReleaseEvent(self, event: QMouseEvent) -> None:
        if self.interactor is None:
            return None

        self.makeCurrent()

        self._update_modifiers(event.modifiers())
        button = self._map_mouse_button(event.button())
        if button is not None:
            inter = self.interactor
            inter.trigger_mouse_button(inter.InputAction.RELEASE, button)
            pos: QPointF = event.position()
            inter.trigger_mouse_position(pos.x(), pos.y())
            self.update()

        self.doneCurrent()
        return None

    def wheelEvent(self, event: QWheelEvent) -> None:
        if self.interactor is None:
            return None

        self.makeCurrent()

        self._update_modifiers(event.modifiers())
        inter = self.interactor

        delta = event.angleDelta()
        if abs(delta.y()) >= abs(delta.x()):
            direction = (
                inter.WheelDirection.FORWARD
                if delta.y() > 0
                else inter.WheelDirection.BACKWARD
            )
        else:
            direction = (
                inter.WheelDirection.RIGHT
                if delta.x() > 0
                else inter.WheelDirection.LEFT
            )

        inter.trigger_mouse_wheel(direction)
        self.update()

        self.doneCurrent()
        return None

    # ------------------------------------------------------------------
    # Keyboard interaction forwarding
    # ------------------------------------------------------------------

    def keyPressEvent(self, event: QKeyEvent) -> None:
        if self.interactor is None:
            return None

        self.makeCurrent()
        self._update_modifiers(event.modifiers())
        inter = self.interactor
        key_sym = self._key_sym_from_event(event)

        if key_sym:
            inter.trigger_keyboard_key(inter.InputAction.PRESS, key_sym)

        text = event.text()
        if text:
            for ch in text:
                inter.trigger_text_character(ord(ch))

        self.update()
        self.doneCurrent()
        return None

    def keyReleaseEvent(self, event: QKeyEvent) -> None:
        if self.interactor is None:
            return None

        self.makeCurrent()
        self._update_modifiers(event.modifiers())
        inter = self.interactor
        key_sym = self._key_sym_from_event(event)

        if key_sym:
            inter.trigger_keyboard_key(inter.InputAction.RELEASE, key_sym)

        self.update()
        self.doneCurrent()
        return None


if __name__ == "__main__":
    raise SystemExit(main())
