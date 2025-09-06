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

template<>
void raw_destructor<f3d::image>(f3d::image* ptr)
{
}
}

f3d::options& toggle(f3d::options& o, const std::string& name)
{
  return o.toggle(name);
}
f3d::options& set_as_string(f3d::options& o, const std::string& name, const std::string& value)
{
  return o.setAsString(name, value);
}
std::string get_as_string(f3d::options& o, const std::string& name)
{
  return o.getAsString(name);
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

f3d::interactor& start(f3d::interactor& inter)
{
  inter.start(1.0 / 30, nullptr);
  return inter;
}

double compare(const f3d::image& current, const f3d::image& other)
{
  return current.compare(other);
}

f3d::image& save(f3d::image& img, const std::string& path)
{
  img.save(std::filesystem::path(path));
  return img;
}

f3d::engine createEngine()
{
  return f3d::engine::create();
}

f3d::image createImage(const std::string& path)
{
  return f3d::image(std::filesystem::path(path));
}

EMSCRIPTEN_BINDINGS(f3d)
{
  // f3d::options
  emscripten::class_<f3d::options>("Options")
    .function("toggle", &toggle, emscripten::return_value_policy::reference())
    .function("set_as_string", &set_as_string, emscripten::return_value_policy::reference())
    .function("get_as_string", &get_as_string, emscripten::return_value_policy::reference())
    .function("set_string", &set_string, emscripten::return_value_policy::reference())
    .function("set_integer", &set_integer, emscripten::return_value_policy::reference())
    .function("set_color", &set_color, emscripten::return_value_policy::reference());

  // f3d::scene
  emscripten::class_<f3d::scene>("Scene")
    .function("supports", &supports, emscripten::return_value_policy::reference())
    .function("add", &add, emscripten::return_value_policy::reference())
    .function("clear", &clear, emscripten::return_value_policy::reference());

  // f3d::image
  emscripten::class_<f3d::image>("Image")
    .class_function("create", &createImage, emscripten::return_value_policy::take_ownership())
    .function("compare", &compare)
    .function("save", &save, emscripten::return_value_policy::reference());

  // f3d::window
  emscripten::class_<f3d::window>("Window")
    .function("setSize", &f3d::window::setSize, emscripten::return_value_policy::reference())
    .function("render", &f3d::window::render)
    .function("renderToImage", &f3d::window::renderToImage)
    .function("resetCamera", &resetCamera, emscripten::return_value_policy::reference());

  // f3d::interactor
  emscripten::class_<f3d::interactor>("Interactor")
    .function("start", &start, emscripten::return_value_policy::reference());

  // f3d::engine
  emscripten::class_<f3d::engine> engine("Engine");

  engine.class_function("create", &createEngine, emscripten::return_value_policy::take_ownership())
    .function("getScene", &f3d::engine::getScene, emscripten::return_value_policy::reference())
    .function("getWindow", &f3d::engine::getWindow, emscripten::return_value_policy::reference())
    .function(
      "getInteractor", &f3d::engine::getInteractor, emscripten::return_value_policy::reference())
    .function("getOptions", &f3d::engine::getOptions, emscripten::return_value_policy::reference())
    .class_function("autoloadPlugins", &f3d::engine::autoloadPlugins);
}
