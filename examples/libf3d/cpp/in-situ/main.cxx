#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/mesh_view.h>
#include <f3d/options.h>
#include <f3d/scene.h>

#include "ClothSolver.h"

namespace
{
void AddCustomCommands(f3d::engine& eng)
{
  // Get the interactor
  f3d::interactor& inter = eng.getInteractor();
  inter.initCommands();

  // Reset options to initial state
  f3d::options& opt = eng.getOptions();
  // Keep a copy of the initial options
  const f3d::options initialOptions = opt;
  inter.addCommand(
    "reset_options",
    [&eng, initialOptions](const std::vector<std::string>&) { eng.getOptions() = initialOptions; },
    f3d::interactor::command_documentation_t{ "reset_options", "Reset basic render/UI options" });

  // Increase animation speed factor
  inter.addCommand(
    "increase_animation_speed_factor",
    [&eng](const std::vector<std::string>&)
    {
      auto& o = eng.getOptions();
      o.scene.animation.speed_factor = f3d::ratio_t(o.scene.animation.speed_factor + 0.05);
    },
    f3d::interactor::command_documentation_t{
      "increase_animation_speed_factor", "Increase animation speed factor" });

  // Decrease animation speed factor
  inter.addCommand(
    "decrease_animation_speed_factor",
    [&eng](const std::vector<std::string>&)
    {
      auto& o = eng.getOptions();
      o.scene.animation.speed_factor = f3d::ratio_t(o.scene.animation.speed_factor - 0.05);
    },
    f3d::interactor::command_documentation_t{
      "decrease_animation_speed_factor", "Decrease animation speed factor" });

  // Toggle grid visibility
  inter.removeCommand("toggle_grid");
  inter.addCommand(
    "toggle_grid",
    [&eng](const std::vector<std::string>&)
    {
      auto& o = eng.getOptions();
      o.render.grid.enable = !o.render.grid.enable;
    },
    f3d::interactor::command_documentation_t{ "toggle_grid", "Toggle ground grid visibility" });

  // Toggle axis
  inter.removeCommand("toggle_axis");
  inter.addCommand(
    "toggle_axis",
    [&eng](const std::vector<std::string>&)
    {
      auto& o = eng.getOptions();
      o.ui.axis = !o.ui.axis;
    },
    f3d::interactor::command_documentation_t{ "toggle_axis", "Toggle axis" });

  // Toggle FXAA anti-aliasing
  inter.removeCommand("toggle_fxaa");
  inter.addCommand(
    "toggle_fxaa",
    [&eng](const std::vector<std::string>&)
    {
      auto& o = eng.getOptions();
      o.render.effect.antialiasing.enable = !o.render.effect.antialiasing.enable;
      // keep mode at fxaa in this example
      o.render.effect.antialiasing.mode = "fxaa";
    },
    f3d::interactor::command_documentation_t{ "toggle_fxaa", "Toggle FXAA anti-aliasing" });

  // Toggle tone mapping
  inter.removeCommand("toggle_tonemapping");
  inter.addCommand(
    "toggle_tonemapping",
    [&eng](const std::vector<std::string>&)
    {
      auto& o = eng.getOptions();
      o.render.effect.tone_mapping = !o.render.effect.tone_mapping;
    },
    f3d::interactor::command_documentation_t{ "toggle_tonemapping", "Toggle tone mapping" });
}

void AddCustomBindins(f3d::engine& eng)
{
  // Get the interactor
  f3d::interactor& inter = eng.getInteractor();
  inter.initBindings();

  f3d::options& opt = eng.getOptions();

  auto docTgl = [](const std::string& doc, const bool& val)
  { return std::pair(doc, (val ? "ON" : "OFF")); };
  auto docStr = [](const std::string& doc) { return std::pair(doc, ""); };
  auto docDblOpt = [](const std::string& doc, const std::optional<double>& val)
  {
    std::stringstream valStream;
    valStream.precision(2);
    valStream << std::fixed;
    if (val.has_value())
    {
      valStream << val.value();
    }
    else
    {
      valStream << "Unset";
    }
    return std::pair(doc, valStream.str());
  };

  // F: toggle FXAA
  inter.removeBinding({ f3d::interaction_bind_t::ModifierKeys::NONE, "F" });
  inter.addBinding(f3d::interaction_bind_t::parse("F"), "toggle render.effect.antialiasing.enable", "Rendering",
    std::bind(docTgl, "Toggle FXAA", std::cref(opt.render.effect.antialiasing.enable)),
    f3d::interactor::BindingType::TOGGLE);
}
}

class ClothMesh : public f3d::mesh_view
{
public:
  explicit ClothMesh(const ClothSolver& solver)
    : Solver(solver)
  {
  }

  std::array<double, 2> getTimeRange() const override
  {
    return { 0.0, 30.0 };
  }

  f3d::mesh_view::memory_view_t getMemoryView(double time) const override
  {
    f3d::mesh_view::memory_view_t view;
    view.pointCount = this->Solver.getPointCount();
    view.points.type = f3d::mesh_view::data_type::F32;
    view.points.data = this->Solver.getPositions();
    view.points.components = 3;
    view.points.stride = 3;

    view.faceOffsetCount = this->Solver.getFaceOffsetCount();
    view.faceOffsets.type = f3d::mesh_view::data_type::U32;
    view.faceOffsets.data = this->Solver.getFaceOffsets();
    view.faceOffsets.components = 1;
    view.faceOffsets.stride = 1;

    view.faceIndexCount = this->Solver.getFaceIndexCount();
    view.faceIndices.type = f3d::mesh_view::data_type::U32;
    view.faceIndices.data = this->Solver.getFaceIndices();
    view.faceIndices.components = 1;
    view.faceIndices.stride = 1;

    view.pointScalars.push_back({ "Mass", f3d::mesh_view::data_type::F32, this->Solver.getInversedMasses(), 1, 1 });
    view.pointScalars.push_back({ "Velocity", f3d::mesh_view::data_type::F32, this->Solver.getVelocities(), 3, 3 });

    return view;
  }

private:
  const ClothSolver& Solver;
};


int main(int argc, char** argv)
{
  // Load static/native plugins
  f3d::engine::autoloadPlugins();

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  // Create a native-window engine
  f3d::engine eng = f3d::engine::create();

  // Modify options using the struct API
  f3d::options& opt = eng.getOptions();
  opt.render.grid.enable = true;
  opt.render.grid.absolute = true;
  opt.render.show_edges = true;

  // UI overlays: axis + some HUD
  opt.ui.axis = true;
  opt.ui.fps = true;
  opt.ui.animation_progress = true;
  opt.ui.filename = true;

  // FXAA + tone mapping
  opt.render.effect.antialiasing.enable = true;
  opt.render.effect.antialiasing.mode = "fxaa";
  opt.render.effect.tone_mapping = true;
  opt.scene.up_direction = { 0.0, 0.0, 1.0 };

  //::AddCustomCommands(eng);
  //::AddCustomBindins(eng);

  ClothSolver solver;

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

  win.getCamera().setPosition({ -9.5f, -8.5f, 8.0f });
  win.getCamera().setFocalPoint({ 0.0f, 0.0f, 3.f });
  win.getCamera().setViewUp({ 0.0f, 0.0f, 1.0f });

  // Start interaction loop
  f3d::interactor& inter = eng.getInteractor();
  if (argc > 2)
  {
    // For testing purposes only, shutdown the example after 1 second
    try
    {
      inter.setEventLoopUserCallback([&inter](f3d::interactor_state_t) { inter.stop(); });
      inter.start(1.0);
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }
  else
  {
    inter.setEventLoopUserCallback([&](f3d::interactor_state_t state) {
      solver.update(state.animationTime);
    });
    inter.start(1.0 / 30.0); // 30 FPS
  }

  return EXIT_SUCCESS;
}
