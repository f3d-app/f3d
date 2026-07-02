#include "PseudoUnitTest.h"

#include <engine.h>
#include <options.h>
#include <scene.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;

using namespace std::string_literals;

int TestSDKStatefile([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  const fs::path dataDir = fs::path(argv[1]) / "data";
  const fs::path tmpDir = argv[2];
  const fs::path cowFile = dataDir / "cow.vtp";
  const fs::path statefilePath = tmpDir / "test_statefile.json";

  // Configure an engine, save its state
  f3d::engine src = f3d::engine::createNone();
  src.getOptions().set("render.background.color", std::vector<double>{ 1.0, 0.0, 0.0 });
  src.getOptions().set("ui.scalar_bar", true);
  src.getScene().add(cowFile);

  test("getAddedFiles returns the added file", src.getScene().getAddedFiles().size() == 1);

  test.expect<f3d::engine::statefile_exception>("save to an invalid path",
    [&]() { src.dump().toFile(tmpDir / "does" / "not" / "exist" / "state.json"); });

  src.dump().toFile(statefilePath);

  // Files outside the statefile directory are stored as absolute paths so the statefile can be
  // loaded without a base directory (e.g. from the standard input or the clipboard)
  std::ifstream statefileStream(statefilePath);
  const std::string statefileContent{ std::istreambuf_iterator<char>(statefileStream),
    std::istreambuf_iterator<char>() };
  test("file outside statefile dir stored as absolute path",
    statefileContent.find(fs::absolute(cowFile).generic_string()) != std::string::npos, true);

  // Restore the state into a fresh engine, check it matches
  f3d::engine dst = f3d::engine::createNone();
  dst.load(f3d::engine::state::fromFile(statefilePath));

  test("restored background color", dst.getOptions().getAsString("render.background.color"),
    src.getOptions().getAsString("render.background.color"));
  test("restored scalar bar option", dst.getOptions().getAsString("ui.scalar_bar"), "true"s);
  test("restored added files", dst.getScene().getAddedFiles().size() == 1);

  // String based round-trip
  const std::string content = src.dump().toString();
  f3d::engine dstStr = f3d::engine::createNone();
  dstStr.load(f3d::engine::state::fromString(content));
  test("string restored background color",
    dstStr.getOptions().getAsString("render.background.color"),
    src.getOptions().getAsString("render.background.color"));
  test("string restored added files", dstStr.getScene().getAddedFiles().size() == 1);

  // clear resets the added files tracking
  dst.getScene().clear();
  test("clear resets added files", dst.getScene().getAddedFiles().empty(), true);

  // Files inside the statefile directory are stored as relative paths and resolved back on load
  const fs::path localCow = tmpDir / "local_cow.vtp";
  fs::copy_file(cowFile, localCow, fs::copy_options::overwrite_existing);
  const fs::path relStatefilePath = tmpDir / "relative_statefile.json";
  f3d::engine relSrc = f3d::engine::createNone();
  relSrc.getScene().add(localCow);
  relSrc.dump().toFile(relStatefilePath);
  std::ifstream relStream(relStatefilePath);
  const std::string relContent{ std::istreambuf_iterator<char>(relStream),
    std::istreambuf_iterator<char>() };
  test("file inside statefile dir stored as relative path",
    relContent.find(R"("local_cow.vtp")") != std::string::npos, true);
  f3d::engine relDst = f3d::engine::createNone();
  relDst.load(f3d::engine::state::fromFile(relStatefilePath));
  test("relative file restored", relDst.getScene().getAddedFiles().size() == 1);

  // Saving a statefile with no added file still produces a valid statefile
  f3d::engine emptyEng = f3d::engine::createNone();
  emptyEng.dump().toFile(tmpDir / "empty_statefile.json");
  test("empty engine has no added file", emptyEng.getScene().getAddedFiles().empty(), true);

  // Loading a statefile without an options entry is valid
  f3d::engine noOptEng = f3d::engine::createNone();
  noOptEng.load(f3d::engine::state::fromString(R"({ "files": [] })"));
  test(
    "no option statefile leaves no added file", noOptEng.getScene().getAddedFiles().empty(), true);

  // Options that do not exist or cannot be parsed are skipped with a warning, not a failure
  f3d::engine optEng = f3d::engine::createNone();
  optEng.load(f3d::engine::state::fromString(R"({ "options": { "not.a.real.option": "1" } })"));
  optEng.load(
    f3d::engine::state::fromString(R"({ "options": { "render.line_width": "not_a_number" } })"));
  test("invalid options are skipped", optEng.getScene().getAddedFiles().empty(), true);

  // Restoring a camera into an engine without a window is skipped with a log, not a failure
  f3d::engine noWinEng = f3d::engine::createNone();
  noWinEng.load(f3d::engine::state::fromString(
    R"({ "camera": { "position": [0.0, 0.0, 1.0], "focal_point": [0.0, 0.0, 0.0], )"
    R"("view_up": [0.0, 1.0, 0.0], "view_angle": 30.0 } })"));
  test("camera without window is skipped", noWinEng.getScene().getAddedFiles().empty(), true);

  // Failure modes: reading a missing or invalid statefile, or parsing invalid content, all throw
  const fs::path invalidStatefilePath = tmpDir / "invalid_statefile.json";
  std::ofstream(invalidStatefilePath) << "{ not valid json";
  test.expect<f3d::engine::statefile_exception>("read a non existent statefile",
    [&]() { f3d::engine::state::fromFile(tmpDir / "no_such_file.json"); });
  test.expect<f3d::engine::statefile_exception>("read an invalid statefile file",
    [&]() { f3d::engine::state::fromFile(invalidStatefilePath); });
  test.expect<f3d::engine::statefile_exception>(
    "parse invalid statefile content", [&]() { f3d::engine::state::fromString("{ not valid json"); });

  return test.result();
}
