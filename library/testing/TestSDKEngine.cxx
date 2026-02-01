#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>

using namespace std::string_literals;

int TestSDKEngine([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  // clang-format off
  // Load all plugins, built or not, already loaded or not
  try { f3d::engine::loadPlugin("native", {argv[3]}); } catch (...) {}
  try { f3d::engine::loadPlugin("alembic", {argv[3]}); } catch (...) {}
  try { f3d::engine::loadPlugin("assimp", {argv[3]}); } catch (...) {}
  try { f3d::engine::loadPlugin("draco", {argv[3]}); } catch (...) {}
  try { f3d::engine::loadPlugin("exodus", {argv[3]}); } catch (...) {} // F3D_DEPRECATED
  try { f3d::engine::loadPlugin("hdf", {argv[3]}); } catch (...) {}
  try { f3d::engine::loadPlugin("occt", {argv[3]}); } catch (...) {}
  try { f3d::engine::loadPlugin("usd", {argv[3]}); } catch (...) {}
  // clang-format on

  // Test different flags combinations that makes sense
  f3d::engine eng0 = f3d::engine::createNone();
  std::ignore = eng0.getScene();

  f3d::engine eng1 = f3d::engine::create();
  std::ignore = eng1.getScene();
  std::ignore = eng1.getWindow();
  std::ignore = eng1.getInteractor();

  f3d::engine eng2 = f3d::engine::create(true);
  std::ignore = eng2.getScene();
  std::ignore = eng2.getWindow();
  std::ignore = eng2.getInteractor();

  // Test option setters
  f3d::options opt;
  opt.model.scivis.cells = true;

  eng0.setOptions(opt);
  test("set options value using f3d::engine::setOptions(const options& opt)",
    eng0.getOptions().model.scivis.cells);

  opt.render.line_width = 1.7;
  eng0.setOptions(std::move(opt));
  test("set options value using f3d::engine::setOptions(options&& opt)",
    eng0.getOptions().render.line_width.value(), 1.7);

  // Test static information methods
  auto libInfo = f3d::engine::getLibInfo();
  test("check libInfo output", libInfo.License, "BSD-3-Clause"s);

  auto readersInfo = f3d::engine::getReadersInfo();
  test("check readersInfo output", !readersInfo.empty());

  // coverage for getPluginList
  test("check getPluginsList for invalid configs",
    f3d::engine::getPluginsList(std::string(argv[1]) + "configs").empty());
  test("check getPluginsList for inexistent config",
    f3d::engine::getPluginsList("inexistent").empty());

  return test.result();
}
