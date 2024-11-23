#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <options.h>

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
    test("Initial group size", inter.getBindingGroups().size() == nGroup);
    test("Initial nBindings Scene", inter.getBindingsForGroup("Scene").size() == nBindingsScene);
    const auto& [doc, val] = inter.getBindingDocumentation("K", f3d::interactor::ModifierKeys::NONE);
    test("Initial doc and val", doc == initDoc && val == initVal);
  }

  {
    // Test invalid args
    test("Initial invalid group", inter.getBindingsForGroup("Invalid").size() == 0);
    const auto& [doc, val] = inter.getBindingDocumentation("Invalid", f3d::interactor::ModifierKeys::NONE);
    test("Initial invalid doc and val", doc == "" && val == "");
  }

  // Remove all bindings
  for (const std::string& group : inter.getBindingGroups())
  {
    for (const auto& [interaction, modifier] : inter.getBindingsForGroup(group))
    {
      inter.removeBinding(interaction, modifier);
    }
  }

  {
    // Test empty state
    test("Empty group size", inter.getBindingGroups().size() == 0);
    test("Empty nBindings Scene", inter.getBindingsForGroup("Scene").size() == 0);
    const auto& [doc, val] = inter.getBindingDocumentation("K", f3d::interactor::ModifierKeys::NONE);
    test("Empty doc and val", doc == "" && val == "");
  }

  // Add a dummy binding
  inter.addBinding("DummyBind", f3d::interactor::ModifierKeys::NONE, "DummyCommand", "DummyGroup", []() ->std::pair<std::string, std::string>{ return std::pair("DummyDoc", "DummyVal"); });

  {
    // Test dummy binding
    test("Dummy group size", inter.getBindingGroups().size() == 1);
    test("Dummy nBindings DummyGroup", inter.getBindingsForGroup("DummyGroup").size() == 1);
    const auto& [doc, val] = inter.getBindingDocumentation("DummyBind", f3d::interactor::ModifierKeys::NONE);
    test("Dummy doc and val", doc == "DummyDoc" && val == "DummyVal");
  }

  return test.result();
}
