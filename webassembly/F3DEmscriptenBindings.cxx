#include <emscripten/bind.h>

#include "engine.h"
#include "interactor.h"
#include "loader.h"
#include "options.h"
#include "window.h"

namespace emscripten
{
namespace internal
{
template<>
void raw_destructor<f3d::loader>(f3d::loader* ptr) {}

template<>
void raw_destructor<f3d::window>(f3d::window* ptr) {}

template<>
void raw_destructor<f3d::interactor>(f3d::interactor* ptr) {}

template<>
void raw_destructor<f3d::options>(f3d::options* ptr) {}
}
}

f3d::options* getOptionsPtr(f3d::engine& e) { return &e.getOptions(); }
f3d::options* toggle(f3d::options& o, const std::string& name) { return &o.toggle(name); }

f3d::loader* getLoaderPtr(f3d::engine& e) { return &e.getLoader(); }
f3d::loader* loadGeometry(f3d::loader& l, const std::string& p) { return &l.loadGeometry(p); }
f3d::loader* loadScene(f3d::loader& l, const std::string& p) { return &l.loadScene(p); }

f3d::window* getWindowPtr(f3d::engine& e) { return &e.getWindow(); }
f3d::window* setSize(f3d::window& win, int w, int h) { return &win.setSize(w, h); }

f3d::interactor* getInteractorPtr(f3d::engine& e) { return &e.getInteractor(); }

std::string getExceptionMessage(intptr_t exceptionPtr) {
  return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
}

EMSCRIPTEN_BINDINGS(f3d)
{
  // utilities
  emscripten::function("getExceptionMessage", &getExceptionMessage);

  // f3d::options
  emscripten::class_<f3d::options>("Options")
    .function("toggle", &toggle, emscripten::allow_raw_pointers());

  // f3d::loader
  emscripten::class_<f3d::loader>("Loader")
    .function("loadGeometry", &loadGeometry, emscripten::allow_raw_pointers())
    .function("loadScene", &loadScene, emscripten::allow_raw_pointers());

  // f3d::window
  emscripten::class_<f3d::window>("Window")
    .function("setSize", &setSize, emscripten::allow_raw_pointers())
    .function("render", &f3d::window::render);

  // f3d::interactor
  emscripten::class_<f3d::interactor>("Interactor")
    .function("start", &f3d::interactor::start);

  // f3d::engine
  emscripten::class_<f3d::engine> engine("Engine");

  engine.constructor<>()
    .function("getLoader", &getLoaderPtr, emscripten::allow_raw_pointers())
    .function("getWindow", &getWindowPtr, emscripten::allow_raw_pointers())
    .function("getInteractor", &getInteractorPtr, emscripten::allow_raw_pointers())
    .function("getOptions", &getOptionsPtr, emscripten::allow_raw_pointers())
    .class_function("autoloadPlugins", &f3d::engine::autoloadPlugins);
}
