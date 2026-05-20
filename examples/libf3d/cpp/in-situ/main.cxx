/**
 * This example demonstrates how to use libf3d in an in-situ context, where the mesh data is not
 * loaded from a file but generated on the fly by a simulation. The example simulates a cloth using
 * a simple PBD model and displays it in libf3d.
 */

#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/mesh_view.h>
#include <f3d/options.h>
#include <f3d/scene.h>

#include "ClothSolver.h"

/**
 * This class implements a f3d::mesh_view that wraps the cloth simulation data provided by
 * ClothSolver. It allows the cloth simulation to be visualized in libf3d without copying the mesh
 * data, by providing direct memory views of the simulation data.
 */
class ClothMesh : public f3d::mesh_view
{
public:
  explicit ClothMesh(const ClothSolver& solver)
    : Solver(solver)
  {
  }

  std::array<double, 2> getTimeRange() const override
  {
    return { 0.0, 10.0 };
  }

  /**
   * Map solver memory view
   */
  f3d::mesh_view::memory_view_t getMemoryView(double time) const override
  {
    f3d::mesh_view::memory_view_t view;
    view.pointCount = this->Solver.getPointCount();
    view.points.type = f3d::mesh_view::data_type::F32;
    view.points.data = this->Solver.getPositions();
    view.points.components = 3;
    view.points.stride = 3;

    view.polygons.offsetCount = this->Solver.getFaceOffsetCount();
    view.polygons.offsets.type = f3d::mesh_view::data_type::U32;
    view.polygons.offsets.data = this->Solver.getFaceOffsets();

    view.polygons.indexCount = this->Solver.getFaceIndexCount();
    view.polygons.indices.type = f3d::mesh_view::data_type::U32;
    view.polygons.indices.data = this->Solver.getFaceIndices();

    view.vertices.offsetCount = this->Solver.getFixedVertexOffsetCount();
    view.vertices.offsets.type = f3d::mesh_view::data_type::U32;
    view.vertices.offsets.data = this->Solver.getFixedVertexOffsets();

    view.vertices.indexCount = this->Solver.getFixedVertexIndexCount();
    view.vertices.indices.type = f3d::mesh_view::data_type::U32;
    view.vertices.indices.data = this->Solver.getFixedVertexIndices();

    view.pointScalars.push_back(
      { "Mass", f3d::mesh_view::data_type::F32, this->Solver.getInversedMasses(), 1, 1 });
    view.pointScalars.push_back(
      { "Velocity", f3d::mesh_view::data_type::F32, this->Solver.getVelocities(), 3, 3 });

    return view;
  }

private:
  const ClothSolver& Solver;
};

int main(int argc, char** argv)
{
  // Load static/native plugins
  f3d::engine::autoloadPlugins();

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO);

  // Create a native-window engine
  f3d::engine eng = f3d::engine::create();

  // Change some options
  f3d::options& opt = eng.getOptions();

  opt.render.grid.enable = true;
  opt.render.grid.absolute = true;
  opt.render.show_edges = true;
  opt.render.effect.antialiasing.enable = true;
  opt.render.effect.antialiasing.mode = "fxaa";
  opt.render.effect.tone_mapping = true;

  opt.ui.axis = true;
  opt.ui.fps = true;
  opt.ui.animation_progress = true;
  opt.ui.scalar_bar = true;

  opt.scene.up_direction = { 0.0, 0.0, 1.0 };

  ClothSolver solver;

  // Get the interactor
  f3d::interactor& inter = eng.getInteractor();

  // Remove unused bindings
  for (const std::string_view& bind : { "W", "Shift+X", "N", "R", "Shift+N", "Shift+H", "V", "I",
         "O", "Shift+A", "Ctrl+Y", "Ctrl+Z" })
  {
    inter.removeBinding(f3d::interaction_bind_t::parse(bind));
  }

  // Commands
  inter.addCommand(
    "reset_simulation",
    [&](const std::vector<std::string>&)
    {
      solver.initialize();
      eng.getScene().loadAnimationTime(0.0);
    },
    f3d::interactor::command_documentation_t{ "reset_simulation", "Reset simulation" });

  inter.addCommand(
    "set_cloth_resolution",
    [&](const std::vector<std::string>& args)
    {
      if (args.size() != 1)
      {
        f3d::log::error("set_cloth_resolution command requires exactly 1 argument");
        return;
      }
      int gridSize = std::stoi(args[0]);
      if (gridSize < 2)
      {
        f3d::log::error("set_cloth_resolution command requires an integer argument greater than 1");
        return;
      }
      solver.setGridSize(gridSize);
    },
    f3d::interactor::command_documentation_t{ "set_cloth_resolution", "Set cloth resolution" });

  inter.addCommand(
    "set_cloth_iterations",
    [&](const std::vector<std::string>& args)
    {
      if (args.size() != 1)
      {
        f3d::log::error("set_cloth_iterations command requires exactly 1 argument");
        return;
      }
      int iterations = std::stoi(args[0]);
      if (iterations < 1)
      {
        f3d::log::error("set_cloth_iterations command requires an integer argument greater than 0");
        return;
      }
      solver.setIterations(iterations);
    },
    f3d::interactor::command_documentation_t{ "set_cloth_iterations", "Set cloth iterations" });

  // Bindings
  inter.addBinding(f3d::interaction_bind_t::parse("W"), "reset_simulation", "Simulation",
    []() { return std::make_pair<std::string, std::string>("Reset simulation", ""); });

  inter.addBinding(f3d::interaction_bind_t::parse("N"),
    { "set_cloth_resolution 10", "reset_simulation" }, "Simulation",
    []() { return std::make_pair<std::string, std::string>("10x10", ""); });

  inter.addBinding(f3d::interaction_bind_t::parse("Shift+N"),
    { "set_cloth_resolution 50", "reset_simulation" }, "Simulation",
    []() { return std::make_pair<std::string, std::string>("50x50", ""); });

  inter.addBinding(f3d::interaction_bind_t::parse("I"), "set_cloth_iterations 10", "Simulation",
    []() { return std::make_pair<std::string, std::string>("10 iterations", ""); });

  inter.addBinding(f3d::interaction_bind_t::parse("O"), "set_cloth_iterations 100", "Simulation",
    []() { return std::make_pair<std::string, std::string>("100 iterations", ""); });

  try
  {
    // Add a model to the scene
    eng.getScene().add(std::make_shared<ClothMesh>(solver));
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }

  // Initial render
  f3d::window& win = eng.getWindow();
  win.setWindowName("libf3d in-situ example");
  win.setSize(1500, 1000);
  win.setPosition(500, 500);
  win.render();

  win.getCamera().setPosition({ -7.0f, -6.0f, 5.0f });
  win.getCamera().setFocalPoint({ 0.3f, 0.3f, 1.5f });
  win.getCamera().setViewUp({ 0.0f, 0.0f, 1.0f });

  // Start interaction loop
  if (argc > 1)
  {
    // For testing purposes only, shutdown the example after 1 second
    try
    {
      inter.setEventLoopUserCallback([&inter](f3d::interactor_state_t) { inter.stop(); });
      inter.start(std::atoi(argv[1]));
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }
  else
  {
    inter.setEventLoopUserCallback(
      [&](f3d::interactor_state_t state) { solver.update(state.animationTime); });
    inter.start(1.0 / 30.0); // 30 FPS
  }

  return EXIT_SUCCESS;
}
