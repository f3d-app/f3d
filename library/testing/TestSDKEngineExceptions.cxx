#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <window.h>

#include <filesystem>

#include <cstdlib>

namespace fs = std::filesystem;

int TestSDKEngineExceptions(int argc, char* argv[])
{
  PseudoUnitTest test;

  {
    f3d::engine eng = f3d::engine::createNone();
    test.expect<f3d::engine::no_window_exception>(
      "get non-existent window", [&]() { std::ignore = eng.getWindow(); });
    test.expect<f3d::engine::no_interactor_exception>(
      "get non-existent interactor", [&]() { std::ignore = eng.getInteractor(); });

    // Test setCachePath error handling
    test.expect<f3d::engine::cache_exception>(
      "set cache path with empty name", [&]() { eng.setCachePath(""); });
    test.expect<f3d::engine::cache_exception>("set cache path with invalid long name",
      [&]() { eng.setCachePath("/" + std::string(257, 'x')); });

    // cover operator=(engine&&)
    eng = f3d::engine::create(false);
    test("engine assignment operator", eng.getWindow().isOffscreen() == false);
  }

#if defined(__linux__) || defined(__FreeBSD__)
  // Test incorrect engine configuration
  test.expect<f3d::context::loading_exception>(
    "create WGL engine on linux", [&]() { std::ignore = f3d::engine::createWGL(); });
  test.expect<f3d::context::loading_exception>("create external WGL engine on linux",
    [&]() { std::ignore = f3d::engine::createExternalWGL(); });
  test.expect<f3d::context::loading_exception>("create external COCOA engine on linux",
    [&]() { std::ignore = f3d::engine::createExternalCOCOA(); });
  test.expect<f3d::context::loading_exception>(
    "create external engine with invalid library", [&]() {
      std::ignore = f3d::engine::createExternal(f3d::context::getSymbol("invalid", "invalid"));
    });
  test.expect<f3d::context::symbol_exception>("create external engine with invalid symbol", [&]() {
    std::ignore = f3d::engine::createExternal(f3d::context::getSymbol("GLX", "invalid"));
  });
#endif

  // Test loadPlugin error handling
  test.expect<f3d::engine::plugin_exception>("load plugin with invalid library",
    [&]() { f3d::engine::loadPlugin(std::string(argv[1]) + "data/invalid.so"); });

  test.expect<f3d::engine::plugin_exception>("load plugin with invalid library from search paths",
    [&]() { f3d::engine::loadPlugin("invalid", { fs::path(std::string(argv[1]) + "data") }); });

  test.expect<f3d::engine::plugin_exception>("load plugin with invalid long name",
    [&]() { f3d::engine::loadPlugin("/" + std::string(257, 'x') + "/file.ext"); });

#if defined(__linux__) || defined(__FreeBSD__)
  // Test error handling without "HOME" set
  unsetenv("HOME");
  test.expect<f3d::engine::cache_exception>(
    "Create engine without HOME set", [&]() { std::ignore = f3d::engine::createNone(); });
#endif

  return test.result();
}
