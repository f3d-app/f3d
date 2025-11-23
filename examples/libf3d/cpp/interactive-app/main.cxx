#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/options.h>
#include <f3d/scene.h>

#include <iostream>
#include <string>
#include <vector>

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

  // R: reset options
  inter.addBinding({ f3d::interaction_bind_t::ModifierKeys::SHIFT, "R" }, "reset_options",
    "Example", std::bind(docStr, "Reset Options"), f3d::interactor::BindingType::OTHER);

  // SHIFT + S: Increase animation speed
  inter.addBinding({ f3d::interaction_bind_t::ModifierKeys::SHIFT, "S" },
    "increase_animation_speed_factor", "Example",
    std::bind(docDblOpt, "Increase animation speed", std::cref(opt.scene.animation.speed_factor)),
    f3d::interactor::BindingType::NUMERICAL);

  // CTRL + S: Decrease animation speed
  inter.addBinding({ f3d::interaction_bind_t::ModifierKeys::CTRL, "S" },
    "decrease_animation_speed_factor", "Example",
    std::bind(docDblOpt, "Decrease animation speed", std::cref(opt.scene.animation.speed_factor)),
    f3d::interactor::BindingType::NUMERICAL);

  // G: toggle grid
  inter.removeBinding({ f3d::interaction_bind_t::ModifierKeys::NONE, "G" });
  inter.addBinding(f3d::interaction_bind_t::parse("G"), "toggle_grid", "Example",
    std::bind(docTgl, "Toggle grid", std::cref(opt.render.grid.enable)),
    f3d::interactor::BindingType::TOGGLE);

  // X: toggle axis
  inter.removeBinding({ f3d::interaction_bind_t::ModifierKeys::NONE, "X" });
  inter.addBinding(f3d::interaction_bind_t::parse("X"), "toggle_axis", "Example",
    std::bind(docTgl, "Toggle axis", std::cref(opt.ui.axis)), f3d::interactor::BindingType::TOGGLE);

  // F: toggle FXAA
  inter.removeBinding({ f3d::interaction_bind_t::ModifierKeys::NONE, "F" });
  inter.addBinding(f3d::interaction_bind_t::parse("F"), "toggle_fxaa", "Example",
    std::bind(docTgl, "Toggle FXAA", std::cref(opt.render.effect.antialiasing.enable)),
    f3d::interactor::BindingType::TOGGLE);

  // T: toggle tone mapping
  inter.removeBinding({ f3d::interaction_bind_t::ModifierKeys::NONE, "T" });
  inter.addBinding(f3d::interaction_bind_t::parse("T"), "toggle_tonemapping", "Example",
    std::bind(docTgl, "Toggle Tone Mapping", std::cref(opt.render.effect.tone_mapping)),
    f3d::interactor::BindingType::TOGGLE);
}
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return EXIT_FAILURE;
  }

  const std::string file = argv[1];

  // Load static/native plugins
  f3d::engine::autoloadPlugins();

  // Create a native-window engine
  f3d::engine eng = f3d::engine::create();

  // Modify options using the struct API
  f3d::options& opt = eng.getOptions();
  opt.render.grid.enable = true;
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

  ::AddCustomCommands(eng);
  ::AddCustomBindins(eng);

  try
  {
    // Add a model to the scene
    eng.getScene().add(file);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }

  // Initial render
  f3d::window& win = eng.getWindow();
  win.render();

  // Start interaction loop
  f3d::interactor& inter = eng.getInteractor();
  if (argc > 2)
  {
    // For testing purposes only, shutdown the example after `timeout` seconds
    try
    {
      int timeout = std::stoi(argv[2]);
      inter.start(timeout, [&inter]() { inter.stop(); });
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }
  else
  {
    inter.start();
  }

  return EXIT_SUCCESS;
}
