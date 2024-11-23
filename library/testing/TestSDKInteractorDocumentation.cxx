#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>

using mod_t = f3d::interaction_bind_t::ModifierKeys;

constexpr int nGroup = 3;
constexpr int nBindingsCamera = 7;
constexpr std::string_view initDoc = "Toggle Orthographic Projection";
constexpr std::string_view initVal = "N/A";

int TestSDKInteractorDocumentation(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::interactor& inter = eng.getInteractor();

  PseudoUnitTest test;

  {
    // Test initial state
    test("Initial group size", inter.getBindGroups().size() == nGroup);
    test("Initial nBindings Camera", inter.getBindsForGroup("Camera").size() == nBindingsCamera);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "5" });
    test("Initial doc and val", doc == initDoc && val == initVal);
  }

  {
    // Test invalid args
    test("Initial invalid group", inter.getBindsForGroup("Invalid").size() == 0);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "Invalid" });
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
    test("Empty nBindings Camera", inter.getBindsForGroup("Camera").size() == 0);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "5" });
    test("Empty doc and val", doc == "" && val == "");
  }

  // Add a dummy binding
  inter.addBinding({ mod_t::ANY, "DummyBind" }, "DummyCommand", "DummyGroup",
    []() -> std::pair<std::string, std::string> { return std::pair("DummyDoc", "DummyVal"); });

  {
    // Test dummy binding
    test("Dummy group size", inter.getBindGroups().size() == 1);
    test("Dummy nBindings DummyGroup", inter.getBindsForGroup("DummyGroup").size() == 1);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "DummyBind" });
    test("Dummy doc and val", doc == "DummyDoc" && val == "DummyVal");
  }

  // Initialize two times
  inter.initBindings();
  inter.initBindings();
  {
    // Test initial state
    test("Initial group size after init", inter.getBindGroups().size() == nGroup);
    test("Initial nBindings Camera after init",
      inter.getBindsForGroup("Camera").size() == nBindingsCamera);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "5" });
    test("Initial doc and val after init", doc == initDoc && val == initVal);
  }

  return test.result();
}
