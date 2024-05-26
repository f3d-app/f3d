import f3d
import tkinter as tk  # Works also on CustomTKinter
from pyopengltk import OpenGLFrame


class Frame(OpenGLFrame):
    # !!!OpenGlFrame requires adding own code to initgl and redraw!!!
    # !!!This solution only renders the F3D viewer, as it uses EXTERNAL, so controls need to be defined!!!
    def __init__(self):
        super().__init__()
        self.mEngine = None

    # Initialize F3D
    def initgl(self):
        self.mEngine = f3d.Engine(f3d.Window.Type.EXTERNAL)
        self.mEngine.loader.load_geometry(f3d.Mesh(
            points=[0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0],
            face_sides=[3],
            face_indices=[0, 1, 2],
            )
        )

    def redraw(self):
        self.mEngine.window.render()


# Create main window and define size, position and title
root = tk.Tk()
root.geometry('640x480+100+100')
root.title('LanPtr3D')
# Create OpenGL instance
F3D = Frame()
F3D.pack(fill=tk.BOTH, expand=tk.YES)

# Run TKinter mainloop
root.mainloop()
