#include "PseudoUnitTest.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <scene.h>

#include <filesystem>

namespace fs = std::filesystem;

using namespace std::string_literals;

int TestSDKStatefile([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  try
  {
    f3d::engine::loadPlugin("native", { argv[3] });
  }
  catch (...)
  {
  }

  const fs::path dataDir = fs::path(argv[1]) / "data";
  const fs::path tmpDir = argv[2];
  const fs::path cowFile = dataDir / "cow.vtp";
  const fs::path statefilePath = tmpDir / "test_statefile.json";

  // Configure an engine, save its state
  f3d::engine src = f3d::engine::createNone();
  src.getOptions().set("render.background.color", std::vector<double>{ 1.0, 0.0, 0.0 });
  src.getOptions().set("ui.scalar_bar", true);
  src.getScene().add(cowFile);

  test("getAddedFiles returns the added file", src.getScene().getAddedFiles().size(),
    static_cast<size_t>(1));

  test.expect<f3d::engine::statefile_exception>("save to an invalid path",
    [&]() { src.saveStatefile(tmpDir / "does" / "not" / "exist" / "state.json"); });

  src.saveStatefile(statefilePath);

  // Restore the state into a fresh engine, check it matches
  f3d::engine dst = f3d::engine::createNone();
  dst.loadStatefile(statefilePath);

  test("restored background color", dst.getOptions().getAsString("render.background.color"),
    src.getOptions().getAsString("render.background.color"));
  test("restored scalar bar option", dst.getOptions().getAsString("ui.scalar_bar"), "true"s);
  test("restored added files", dst.getScene().getAddedFiles().size(), static_cast<size_t>(1));

  // String based round-trip
  const std::string content = src.saveStatefileToString();
  f3d::engine dstStr = f3d::engine::createNone();
  dstStr.loadStatefileFromString(content);
  test("string restored background color",
    dstStr.getOptions().getAsString("render.background.color"),
    src.getOptions().getAsString("render.background.color"));
  test("string restored added files", dstStr.getScene().getAddedFiles().size(),
    static_cast<size_t>(1));

  // clear resets the added files tracking
  dst.getScene().clear();
  test("clear resets added files", dst.getScene().getAddedFiles().empty(), true);

  // Failure modes
  test.expect<f3d::engine::statefile_exception>(
    "load a non existent statefile", [&]() { dst.loadStatefile(tmpDir / "no_such_file.json"); });
  test.expect<f3d::engine::statefile_exception>(
    "load invalid statefile content", [&]() { dst.loadStatefileFromString("{ not valid json"); });

  return test.result();
}
