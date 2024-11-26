#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>

using mod_t = f3d::interaction_bind_t::ModifierKeys;

constexpr int nGroup = 3;
constexpr int nBindsCamera = 7;
constexpr std::string_view initDoc = "Toggle Orthographic Projection";
constexpr std::string_view initVal = "N/A";

int TestSDKInteractorDocumentation(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::interactor& inter = eng.getInteractor();

  // Avoid testing something that changes often
  int nBinds = inter.getBinds().size();

  PseudoUnitTest test;

  {
    // Test initial state
    test("Initial group size", inter.getBindGroups().size() == nGroup);
    test("Initial nBinds Camera", inter.getBindsForGroup("Camera").size() == nBindsCamera);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "5" });
    test("Initial doc and val", doc == initDoc && val == initVal);
  }

  {
    // check exceptions for invalid args
    test.expect<f3d::interactor::does_not_exists_exception>(
      "Initial invalid group", [&]() { inter.getBindsForGroup("Invalid"); });

    test.expect<f3d::interactor::does_not_exists_exception>(
      "Initial invalid bind", [&]() { inter.getBindingDocumentation({ mod_t::ANY, "Invalid" }); });
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
    test("Empty binds size", inter.getBinds().size() == 0);
    // check exceptions for invalid args
    test.expect<f3d::interactor::does_not_exists_exception>(
      "Empty group", [&]() { inter.getBindsForGroup("Camera"); });

    test.expect<f3d::interactor::does_not_exists_exception>(
      "Empty bind", [&]() { inter.getBindingDocumentation({ mod_t::ANY, "5" }); });
  }

  // Add a dummy binding
  inter.addBinding({ mod_t::ANY, "DummyBind" }, "DummyCommand", "DummyGroup",
    []() -> std::pair<std::string, std::string> { return std::pair("DummyDoc", "DummyVal"); });

  {
    // Test dummy binding
    test("Dummy group size", inter.getBindGroups().size() == 1);
    test("Dummy binds size", inter.getBinds().size() == 1);
    test("Dummy nBinds DummyGroup", inter.getBindsForGroup("DummyGroup").size() == 1);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "DummyBind" });
    test("Dummy doc and val", doc == "DummyDoc" && val == "DummyVal");
  }

  // Initialize two times
  inter.initBindings();
  inter.initBindings();
  {
    // Test initial state
    test("Initial group size after init", inter.getBindGroups().size() == nGroup);
    test("Initial binds size", inter.getBinds().size() == nBinds);
    test(
      "Initial nBinds Camera after init", inter.getBindsForGroup("Camera").size() == nBindsCamera);
    const auto& [doc, val] = inter.getBindingDocumentation({ mod_t::ANY, "5" });
    test("Initial doc and val after init", doc == initDoc && val == initVal);
  }

  return test.result();
}
