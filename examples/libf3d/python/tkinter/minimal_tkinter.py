import tkinter as tk  # Works also on CustomTKinter
from argparse import ArgumentParser

import f3d
from pyopengltk import OpenGLFrame


class Frame(OpenGLFrame):
    # !!!OpenGlFrame requires adding own code to initgl and redraw!!!
    # !!!This solution only renders the F3D viewer, as it uses EXTERNAL, so controls need to be defined!!!
    def __init__(self):
        super().__init__()
        self.engine = None

    # Initialize F3D
    def initgl(self):
        self.engine = (
            f3d.Engine.create_external_glx()
        )  # use create_external_egl for Wayland
        self.engine.scene.add(
            f3d.Mesh(
                points=[0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0],
                face_sides=[3],
                face_indices=[0, 1, 2],
            )
        )

    def redraw(self):
        self.engine.window.render()


def main(argv: list[str] | None = None):
    argparser = ArgumentParser()
    argparser.add_argument(
        "--timeout",
        type=int,
        default=None,
        help="Optional timeout (in seconds) before closing the viewer.",
    )

    args = argparser.parse_args(argv)

    # Create main window and define size, position and title
    root = tk.Tk()
    root.geometry("640x480+100+100")
    root.title("Minimal tkinter interface")
    # Create OpenGL instance
    F3D = Frame()
    F3D.pack(fill=tk.BOTH, expand=tk.YES)

    if args.timeout:
        # For testing purposes only, exit after `timeout` seconds
        root.after(args.timeout * 1000, root.destroy)
    # Run TKinter mainloop
    root.mainloop()


if __name__ == "__main__":
    main()
