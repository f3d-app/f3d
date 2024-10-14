#include <emscripten/bind.h>

#include "engine.h"
#include "interactor.h"
#include "options.h"
#include "scene.h"
#include "window.h"

namespace emscripten::internal
{
template<>
void raw_destructor<f3d::scene>(f3d::scene* ptr)
{
}

template<>
void raw_destructor<f3d::window>(f3d::window* ptr)
{
}

template<>
void raw_destructor<f3d::interactor>(f3d::interactor* ptr)
{
}

template<>
void raw_destructor<f3d::options>(f3d::options* ptr)
{
}
}

f3d::options& toggle(f3d::options& o, const std::string& name)
{
  return o.toggle(name);
}
f3d::options& set_string(f3d::options& o, const std::string& name, const std::string& value)
{
  return o.set(name, value);
}
f3d::options& set_integer(f3d::options& o, const std::string& name, int value)
{
  return o.set(name, value);
}
f3d::options& set_color(f3d::options& o, const std::string& name, double r, double g, double b)
{
  return o.set(name, std::vector{ r, g, b });
}

f3d::scene& getScenePtr(f3d::engine& e)
{
  return e.getScene();
}
f3d::scene& add(f3d::scene& l, const std::string& p)
{
  return l.add(p);
}
bool supports(f3d::scene& l, const std::string& p)
{
  return l.supports(p);
}
f3d::scene* clear(f3d::scene& l)
{
  return &l.clear();
}

f3d::window& resetCamera(f3d::window& win)
{
  win.getCamera().resetToBounds();
  return win;
}

f3d::engine createEngine()
{
  return f3d::engine::create();
}

EMSCRIPTEN_BINDINGS(f3d)
{
  // f3d::options
  emscripten::class_<f3d::options>("Options")
    .function("toggle", &toggle, emscripten::return_value_policy::reference())
    .function("set_string", &set_string, emscripten::return_value_policy::reference())
    .function("set_integer", &set_integer, emscripten::return_value_policy::reference())
    .function("set_color", &set_color, emscripten::return_value_policy::reference());

  // f3d::scene
  emscripten::class_<f3d::scene>("Scene")
    .function("supports", &supports, emscripten::return_value_policy::reference())
    .function("add", &add, emscripten::return_value_policy::reference())
    .function("clear", &clear, emscripten::return_value_policy::reference());

  // f3d::window
  emscripten::class_<f3d::window>("Window")
    .function("setSize", &f3d::window::setSize, emscripten::return_value_policy::reference())
    .function("render", &f3d::window::render)
    .function("resetCamera", &resetCamera, emscripten::return_value_policy::reference());

  // f3d::interactor
  emscripten::class_<f3d::interactor>("Interactor").function("start", &f3d::interactor::start);

  // f3d::engine
  emscripten::class_<f3d::engine> engine("Engine");

  engine
    .class_function("create", &createEngine, emscripten::return_value_policy::take_ownership())
    .function("getScene", &f3d::engine::getScene, emscripten::return_value_policy::reference())
    .function("getWindow", &f3d::engine::getWindow, emscripten::return_value_policy::reference())
    .function("getInteractor", &f3d::engine::getInteractor, emscripten::return_value_policy::reference())
    .function("getOptions", &f3d::engine::getOptions, emscripten::return_value_policy::reference())
    .class_function("autoloadPlugins", &f3d::engine::autoloadPlugins);
}
