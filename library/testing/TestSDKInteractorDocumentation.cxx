#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <options.h>

using mod_t = f3d::interaction_bind_t::ModifierKeys;

constexpr int nGroup = 3;
constexpr int nBindingsScene = 26;
constexpr std::string_view initDoc = "Toggle trackball interaction";
constexpr std::string_view initVal = "OFF";

int TestSDKInteractorDocumentation(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::options& options = eng.getOptions();
  f3d::interactor& inter = eng.getInteractor();

  PseudoUnitTest test;

  {
    // Test initial state
    test("Initial group size", inter.getBindGroups().size() == nGroup);
    test("Initial nBindings Scene", inter.getBindsForGroup("Scene").size() == nBindingsScene);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::NONE, "K" });
    test("Initial doc and val", doc == initDoc && val == initVal);
  }

  {
    // Test invalid args
    test("Initial invalid group", inter.getBindsForGroup("Invalid").size() == 0);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::NONE, "Invalid" });
    test("Initial invalid doc and val", doc == "" && val == "");
  }

  // Remove all bindings
  for (const std::string& group : inter.getBindGroups())
  {
    for (const f3d::interaction_bind_t& bind : inter.getBindsForGroup(group))
    {
      inter.removeBinding(bind);
    }
  }

  {
    // Test empty state
    test("Empty group size", inter.getBindGroups().size() == 0);
    test("Empty nBindings Scene", inter.getBindsForGroup("Scene").size() == 0);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::NONE, "K" });
    test("Empty doc and val", doc == "" && val == "");
  }

  // Add a dummy binding
  inter.addBinding({ mod_t::NONE, "DummyBind" }, "DummyCommand", "DummyGroup",
    []() -> std::pair<std::string, std::string> { return std::pair("DummyDoc", "DummyVal"); });

  {
    // Test dummy binding
    test("Dummy group size", inter.getBindGroups().size() == 1);
    test("Dummy nBindings DummyGroup", inter.getBindsForGroup("DummyGroup").size() == 1);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::NONE, "DummyBind" });
    test("Dummy doc and val", doc == "DummyDoc" && val == "DummyVal");
  }

  return test.result();
}
