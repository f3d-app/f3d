"""
This example demonstrates how to use libf3d in an in-situ context, where the mesh data is not
loaded from a file but generated on the fly by a simulation. The example simulates a cloth using
a simple [PBD](https://en.wikipedia.org/wiki/Soft-body_dynamics#Position-based_dynamics) model and displays it in libf3d.
"""

import math
from argparse import ArgumentParser

import numpy as np

import f3d


class ClothSolver:
    """Simple cloth solver based on the PBD method."""

    def __init__(self, grid_size: int = 10, iterations: int = 20):
        self._grid_size = grid_size
        self._iterations = iterations
        self._current_time = 0.0

        self.initialize()

    def set_grid_size(self, grid_size: int) -> None:
        self._grid_size = grid_size

    def set_iterations(self, iterations: int) -> None:
        self._iterations = iterations

    def initialize(self) -> None:
        self._current_time = 0.0
        n = self._grid_size
        edge_len = 2.0 / float(n)
        n_pts = (n + 1) * (n + 1)

        # Build grid of (n+1) x (n+1) points at z=3
        pos = np.empty((n_pts, 3), dtype=np.float32)
        for i in range(n + 1):
            for j in range(n + 1):
                idx = i * (n + 1) + j
                pos[idx, 0] = -1.0 + i * edge_len
                pos[idx, 1] = -1.0 + j * edge_len
                pos[idx, 2] = 3.0

        self.positions = pos
        self.inversed_masses = np.ones(n_pts, dtype=np.float32)
        self.velocities = np.zeros((n_pts, 3), dtype=np.float32)
        self._next_positions = self.positions.copy()

        # Fixed vertices: corners at index 0 and index n (top-left, top-right of the grid)
        self.fixed_vertex_indices = np.array([0, n], dtype=np.int32)
        self.fixed_vertex_offsets = np.array([0, 1, 2], dtype=np.int32)

        self.inversed_masses[0] = 0.0
        self.inversed_masses[n] = 0.0

        # Build faces and distance constraints
        face_indices_list = []
        face_offsets_list = [0]
        constraints = []  # list of (p1, p2, rest_length)

        diag_len = math.sqrt(2.0) * edge_len

        for i in range(n):
            for j in range(n):
                top_left = (i + 1) * (n + 1) + j
                top_right = top_left + 1
                bottom_left = i * (n + 1) + j
                bottom_right = bottom_left + 1

                face_indices_list.extend(
                    [top_right, bottom_right, bottom_left, top_left]
                )
                face_offsets_list.append(len(face_indices_list))

                # Avoid duplicate constraints: only top and left edges of each quad
                constraints.append((top_left, top_right, edge_len))
                constraints.append((top_left, bottom_left, edge_len))

                # Right edge of last column
                if j == n - 1:
                    constraints.append((top_right, bottom_right, edge_len))
                # Bottom edge of last row
                if i == n - 1:
                    constraints.append((bottom_left, bottom_right, edge_len))

                # Diagonal constraints for stability
                constraints.append((top_left, bottom_right, diag_len))
                constraints.append((top_right, bottom_left, diag_len))

        self.face_indices = np.array(face_indices_list, dtype=np.int32)
        self.face_offsets = np.array(face_offsets_list, dtype=np.int32)
        self._constraints = constraints

    def update(self, new_time: float) -> None:
        if new_time == 0.0:
            if self._current_time != 0.0:
                self.initialize()
            return

        time_step = new_time - self._current_time

        if time_step == 0.0:
            return

        if time_step < 0.0:
            self.initialize()
            return

        self._current_time = new_time
        gravity = -9.81

        # Apply gravity on Z axis and predict next positions
        self.velocities[:, 2] += gravity * self.inversed_masses * time_step
        np.add(self.positions, self.velocities * time_step, out=self._next_positions)

        # Project distance constraints
        for _ in range(self._iterations):
            for p1, p2, rest_length in self._constraints:
                d = self._next_positions[p2] - self._next_positions[p1]
                length = float(np.linalg.norm(d))
                if length < 1e-10:
                    continue
                diff = (length - rest_length) / length
                inv1 = float(self.inversed_masses[p1])
                inv2 = float(self.inversed_masses[p2])
                sum_inv = inv1 + inv2
                if sum_inv > 0.0:
                    c1 = (inv1 / sum_inv) * diff
                    c2 = (inv2 / sum_inv) * diff
                    self._next_positions[p1] += c1 * d
                    self._next_positions[p2] -= c2 * d

        # Update velocities and positions
        self.velocities[:] = (self._next_positions - self.positions) / time_step
        np.copyto(self.positions, self._next_positions)


class ClothMesh(f3d.MeshView):
    """
    A f3d.MeshView wrapping the cloth simulation data provided by ClothSolver. It allows the cloth
    simulation to be visualized in libf3d without copying the mesh data, by providing direct memory
    views of the simulation arrays.
    """

    def __init__(self, solver: ClothSolver):
        super().__init__()
        self._solver = solver

    def get_time_range(self) -> list:
        return [0.0, 10.0]

    def get_memory_view(self, time: float) -> f3d.MeshMemoryView:
        view = f3d.MeshMemoryView()
        view.points = self._solver.positions
        view.polygons_offsets = self._solver.face_offsets
        view.polygons_indices = self._solver.face_indices
        view.vertices_offsets = self._solver.fixed_vertex_offsets
        view.vertices_indices = self._solver.fixed_vertex_indices
        view.point_scalars = {
            "Mass": self._solver.inversed_masses,
            "Velocity": self._solver.velocities,
        }
        return view


def main(argv=None):
    parser = ArgumentParser()
    parser.add_argument(
        "--timeout",
        type=int,
        help="Optional timeout (in seconds) before closing the viewer.",
    )
    args = parser.parse_args(argv)

    f3d.Engine.autoload_plugins()
    f3d.Log.set_verbose_level(f3d.Log.VerboseLevel.INFO)

    # Create a native-window engine
    eng = f3d.Engine.create()

    # Change some options
    opt = eng.options
    opt.update(
        {
            "render.grid.enable": True,
            "render.grid.absolute": True,
            "render.show_edges": True,
            "render.effect.antialiasing.enable": True,
            "render.effect.antialiasing.mode": "fxaa",
            "render.effect.tone_mapping": True,
            "ui.axis": True,
            "ui.fps": True,
            "ui.animation_progress": True,
            "ui.scalar_bar": True,
            "scene.up_direction": [0.0, 0.0, 1.0],
        }
    )

    solver = ClothSolver()
    inter = eng.interactor

    # Remove bindings that conflict with our custom ones or are unused in this example
    NONE = f3d.InteractionBind.ModifierKeys.NONE
    SHIFT = f3d.InteractionBind.ModifierKeys.SHIFT
    CTRL = f3d.InteractionBind.ModifierKeys.CTRL
    for mod, key in [
        (NONE, "W"),
        (SHIFT, "X"),
        (NONE, "N"),
        (NONE, "R"),
        (SHIFT, "N"),
        (SHIFT, "H"),
        (NONE, "V"),
        (NONE, "I"),
        (NONE, "O"),
        (SHIFT, "A"),
        (CTRL, "Y"),
        (CTRL, "Z"),
    ]:
        inter.remove_binding(f3d.InteractionBind(mod, key))

    # Commands
    def reset_simulation(_args):
        solver.initialize()
        scene.load_animation_time(0.0)

    inter.add_command(
        "reset_simulation",
        reset_simulation,
        ("reset_simulation", "Reset simulation"),
    )

    def set_cloth_resolution(args):
        if len(args) != 1:
            f3d.Log.print(
                f3d.Log.ERROR,
                "set_cloth_resolution command requires exactly 1 argument",
            )
            return
        try:
            grid_size = int(args[0])
        except ValueError:
            f3d.Log.print(
                f3d.Log.ERROR,
                "set_cloth_resolution command requires an integer argument",
            )
            return
        if grid_size < 2:
            f3d.Log.print(
                f3d.Log.ERROR,
                "set_cloth_resolution command requires an integer argument greater than 1",
            )
            return
        solver.set_grid_size(grid_size)

    inter.add_command(
        "set_cloth_resolution",
        set_cloth_resolution,
        ("set_cloth_resolution", "Set cloth resolution"),
    )

    def set_cloth_iterations(args):
        if len(args) != 1:
            f3d.Log.print(
                f3d.Log.ERROR,
                "set_cloth_iterations command requires exactly 1 argument",
            )
            return
        try:
            iterations = int(args[0])
        except ValueError:
            f3d.Log.print(
                f3d.Log.ERROR,
                "set_cloth_iterations command requires an integer argument",
            )
            return
        if iterations < 1:
            f3d.Log.print(
                f3d.Log.ERROR,
                "set_cloth_iterations command requires an integer argument greater than 0",
            )
            return
        solver.set_iterations(iterations)

    inter.add_command(
        "set_cloth_iterations",
        set_cloth_iterations,
        ("set_cloth_iterations", "Set cloth iterations"),
    )

    # Bindings
    inter.add_binding(
        f3d.InteractionBind(NONE, "W"),
        "reset_simulation",
        "Simulation",
        lambda: ("Reset simulation", ""),
    )
    inter.add_binding(
        f3d.InteractionBind(NONE, "N"),
        ["set_cloth_resolution 10", "reset_simulation"],
        "Simulation",
        lambda: ("10x10", ""),
    )
    inter.add_binding(
        f3d.InteractionBind(SHIFT, "N"),
        ["set_cloth_resolution 20", "reset_simulation"],
        "Simulation",
        lambda: ("20x20", ""),
    )
    inter.add_binding(
        f3d.InteractionBind(NONE, "I"),
        "set_cloth_iterations 10",
        "Simulation",
        lambda: ("10 iterations", ""),
    )
    inter.add_binding(
        f3d.InteractionBind(NONE, "O"),
        "set_cloth_iterations 20",
        "Simulation",
        lambda: ("20 iterations", ""),
    )

    # Add the cloth mesh to the scene
    cloth_mesh = ClothMesh(solver)
    try:
        eng.scene.add(cloth_mesh)
    except Exception as e:
        print(e)
        return 1

    # Initial render
    win = eng.window
    win.set_window_name("libf3d in-situ example")
    win.size = (1500, 1000)
    win.set_position(500, 500)
    win.render()

    cam = win.camera
    cam.position = [-7.0, -6.0, 5.0]
    cam.focal_point = [0.3, 0.3, 1.5]
    cam.view_up = [0.0, 0.0, 1.0]

    if args.timeout:
        # For testing purposes only, exit after `timeout` seconds
        inter.set_event_loop_user_callback(lambda _state: inter.request_stop())
        inter.start(args.timeout)
    else:
        inter.set_event_loop_user_callback(
            lambda state: solver.update(state.animation_time)
        )
        inter.start(1.0 / 30.0)  # 30 FPS


if __name__ == "__main__":
    import sys

    sys.exit(main())
