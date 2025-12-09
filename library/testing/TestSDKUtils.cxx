#include "PseudoUnitTest.h"

#include <utils.h>

#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

int TestSDKUtils([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  test("tokenize spaces",
    f3d::utils::tokenize(R"(set render.hdri.file file path space)") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file", "path", "space" });

  test("tokenize double quotes",
    f3d::utils::tokenize(R"(set render.hdri.file "file path double")") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file path double" });

  test("tokenize single quotes",
    f3d::utils::tokenize(R"(set render.hdri.file 'file path single')") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file path single" });

  test("tokenize backticks",
    f3d::utils::tokenize(R"(set render.hdri.file `file path back`)") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file path back" });

  test("tokenize double quotes with internal quotes",
    f3d::utils::tokenize(R"(set render.hdri.file "file 'path' double")") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file 'path' double" });

  test("tokenize single quotes with internal quotes",
    f3d::utils::tokenize(R"(set render.hdri.file 'file "path" single')") ==
      std::vector<std::string>{ "set", "render.hdri.file", R"(file "path" single)" });

  test("tokenize backticks quotes with internal quotes",
    f3d::utils::tokenize(R"(set render.hdri.file `file "path" back`)") ==
      std::vector<std::string>{ "set", "render.hdri.file", R"(file "path" back)" });

  test("tokenize double quotes with escape",
    f3d::utils::tokenize(R"(set render.hdri.file "file \"path\" double")") ==
      std::vector<std::string>{ "set", "render.hdri.file", R"(file "path" double)" });

  test("tokenize single quotes with escape",
    f3d::utils::tokenize(R"(set render.hdri.file 'file \'path\' single')") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file 'path' single" });

  test("tokenize backticks quotes with escape",
    f3d::utils::tokenize(R"(set render.hdri.file `file \`path\` single`)") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file `path` single" });

  test("tokenize spaces with escape",
    f3d::utils::tokenize(R"(set render.hdri.file file\ pa\th\ esc\ape)") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file path escape" });

  test("tokenize comments",
    f3d::utils::tokenize(R"(set render.hdri.file file # A comment)") ==
      std::vector<std::string>{ "set", "render.hdri.file", "file" });

  test("tokenize comments with keep comments flag",
    f3d::utils::tokenize(R"(set render.hdri.file file # A comment)", false) ==
      std::vector<std::string>{ "set", "render.hdri.file", "file", "#", "A", "comment" });

  test("tokenize base color",
    f3d::utils::tokenize(R"(set render.background.color #000)") ==
      std::vector<std::string>{ "set", "render.background.color" });

  test("tokenize base color with keep comments flag",
    f3d::utils::tokenize(R"(set render.background.color #000)", false) ==
      std::vector<std::string>{ "set", "render.background.color", "#000" });

  test("tokenize escaped comments",
    f3d::utils::tokenize(R"(set render.hdri.file fi\#le)") ==
      std::vector<std::string>{ "set", "render.hdri.file", "fi#le" });

  test("tokenize backslashes",
    f3d::utils::tokenize(R"(set render.hdri.file file\\pa\\th\\backsl\\ashes)") ==
      std::vector<std::string>{ "set", "render.hdri.file", R"(file\pa\th\backsl\ashes)" });

  test.expect<f3d::utils::tokenize_exception>("tokenize_exception with incomplete quotes",
    [&]() { std::ignore = f3d::utils::tokenize(R"(set render.hdri.file "file path back)"); });

  test.expect<f3d::utils::tokenize_exception>("tokenize_exception with unfinishied escape",
    [&]() { std::ignore = f3d::utils::tokenize(R"(set render.hdri.file file path back\)"); });

  //

  test("string_template: basic substitution",
    f3d::utils::string_template("{greeting} {name}!")
      .substitute({ { "greeting", "hello" }, { "name", "World" } })
      .str(),
    "hello World!");

  test("string_template: partial substitution",
    f3d::utils::string_template("{greeting} {name}!").substitute({ { "greeting", "hello" } }).str(),
    "hello {name}!");

  test("string_template: multi-step substitution",
    f3d::utils::string_template("{greeting} {name}!")
      .substitute({ { "greeting", "hello" } })
      .substitute({ { "name", "World" } })
      .str(),
    "hello World!");

  test("string_template: escaped variable substitution",
    f3d::utils::string_template("{greeting} {{name}}!")
      .substitute({ { "greeting", "hello" } })
      .substitute({ { "name", "World" } })
      .str(),
    "hello {name}!");

  test("string_template: non-recursive substitution",
    f3d::utils::string_template("{greeting} {name}!")
      .substitute({ { "greeting", "hello" }, { "name", "{foo}" } })
      .substitute({ { "foo", "bar" } })
      .str(),
    "hello {foo}!");

  //

  test("collapsePath: empty", f3d::utils::collapsePath("").empty());
  test("collapsePath: relative to absolute",
    f3d::utils::collapsePath("folder/file.ext").is_absolute());
  test("collapsePath: relative with folder",
    f3d::utils::collapsePath("folder/file.ext", "/") == "/folder/file.ext");
  test(
    "collapsePath: remove dotdot", f3d::utils::collapsePath("/folder/../file.ext") == "/file.ext");
  test("collapsePath: expand home", f3d::utils::collapsePath("~/folder/file.ext").is_absolute());

  //

  auto globMatchesText = [](std::string_view glob, const std::string& text,
                           char pathSeparator = '/') {
    std::string regexPattern = f3d::utils::globToRegex(glob, pathSeparator);
    std::regex regex(regexPattern);
    return std::regex_match(text, regex);
  };
  test("globToRegex: exact match", globMatchesText("file.txt", "file.txt"));
  test("globToRegex: simple star", globMatchesText("*.txt", "file.txt"));
  test("globToRegex: simple star - no match", !globMatchesText("*.txt", "file.txt.bak"));
  test("globToRegex: simple question mark", globMatchesText("file?.txt", "file1.txt"));
  test("globToRegex: simple question mark - no match", !globMatchesText("file?.txt", "file12.txt"));
  test("globToRegex: character class - match digit", globMatchesText("file[0-9].txt", "file5.txt"));
  test(
    "globToRegex: character class - match letter", globMatchesText("file[a-z].txt", "filex.txt"));
  test("globToRegex: character class - no match", !globMatchesText("file[a-z].txt", "file5.txt"));
  test("globToRegex: negated character class", globMatchesText("file[!0-9].txt", "filex.txt"));
  test("globToRegex: negated character class - no match",
    !globMatchesText("file[!0-9].txt", "file5.txt"));
  test("globToRegex: character class with special chars",
    globMatchesText("file[*?].txt", "file*.txt"));
  test("globToRegex: character class with character class opening brace",
    globMatchesText("file[[].txt", "file[.txt"));
  test("globToRegex: simple alternation", globMatchesText("file.{txt,md,cpp}", "file.txt"));
  test("globToRegex: simple alternation - second option",
    globMatchesText("file.{txt,md,cpp}", "file.md"));
  test(
    "globToRegex: simple alternation - no match", !globMatchesText("file.{txt,md,cpp}", "file.py"));
  test("globToRegex: nested alternation", globMatchesText("{a,b{c,d}}.txt", "a.txt"));
  test(
    "globToRegex: nested alternation - inner match", globMatchesText("{a,b{c,d}}.txt", "bc.txt"));
  test("globToRegex: false alternation in character class",
    globMatchesText("file[{1,2,3}].txt", "file1.txt"));
  test("globToRegex: escape period", globMatchesText("file\\.txt", "file\\.txt"));
  test("globToRegex: escape period - no match", !globMatchesText("file\\.txt", "file.txt"));
  test("globToRegex: escaped escape", globMatchesText("dir\\\\file.txt", "dir\\file.txt"));
  test("globToRegex: special regex chars", globMatchesText("file+.txt", "file+.txt"));
  test("globToRegex: special regex chars", globMatchesText("file(test).txt", "file(test).txt"));
  test("globToRegex: complex pattern 1",
    globMatchesText("*[0-9]_[a-z]*.{jpg,png}", "image5_photo.jpg"));
  test(
    "globToRegex: complex pattern 2", !globMatchesText("*[0-9]_[a-z]*.{jpg,png}", "pic_photo.jpg"));
  test("globToRegex: complex pattern 3",
    globMatchesText("log_[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9].txt", "log_2025-05-18.txt"));
  test("globToRegex: complex pattern 4",
    globMatchesText("data_?{_v[0-9],}.{csv,dat}", "data_1_v2.csv"));
  test("globToRegex: only special chars", globMatchesText("***??[a-z][0-9]", "abc5"));
  test("globToRegex: empty glob", globMatchesText("", ""));
  test("globToRegex: globstar simple", globMatchesText("**/file.txt", "dir/file.txt"));
  test("globToRegex: globstar simple - Windows separator",
    globMatchesText("**\\\\file.txt", "dir\\file.txt", '\\'));
  test("globToRegex: globstar deep", globMatchesText("**/file.txt", "dir1/dir2/dir3/file.txt"));
  test("globToRegex: globstar deep - Windows separator",
    globMatchesText("**\\\\file.txt", R"(dir1\dir2\dir3\file.txt)", '\\'));
  test(
    "globToRegex: globstar middle 1", globMatchesText("src/**/*.cxx", "src/library/src/file.cxx"));
  test("globToRegex: globstar middle 1 - Windows separator",
    globMatchesText(R"(src\\**\\*.cxx)", R"(src\library\src\file.cxx)", '\\'));
  test("globToRegex: globstar middle 2",
    globMatchesText("src/**/include/*.h", "src/external/submodule/include/file.h"));
  test("globToRegex: globstar middle 2 - Windows separator",
    globMatchesText(R"(src\\**\\include\\*.h)", R"(src\external\submodule\include\file.h)", '\\'));
  test("globToRegex: single star", globMatchesText("*", "/anything/"));
  test("globToRegex: two stars", globMatchesText("**", "/anything/"));
  test("globToRegex: three stars", globMatchesText("***", "/anything/"));
  test("globToRegex: no stars escaped",
    globMatchesText("dir1/***/utils.cxx", "dir1/dir2/dir3/utils.cxx"));
  test("globToRegex: no stars escaped - Windows separator",
    globMatchesText(R"(dir1\\***\\utils.cxx)", R"(dir1\dir2\dir3\utils.cxx)", '\\'));
  test("globToRegex: front star escaped",
    globMatchesText("dir1/\\***/utils.cxx", "dir1/*dir2/utils.cxx"));
  test("globToRegex: front star escaped - Windows separator",
    globMatchesText(R"(dir1\\\***\\utils.cxx)", R"(dir1\*dir2\utils.cxx)", '\\'));
  test("globToRegex: front star escaped - no match",
    !globMatchesText("dir1/\\***/utils.cxx", "dir1/*dir2/dir3/utils.cxx"));
  test("globToRegex: front star escaped - no match - Windows separator",
    !globMatchesText(R"(dir1\\\***\\utils.cxx)", R"(dir1\*dir2\dir3\utils.cxx)", '\\'));
  test("globToRegex: middle star escaped",
    globMatchesText("dir1/*\\**/utils.cxx", "dir1/anything*anything/utils.cxx"));
  test("globToRegex: middle star escaped - Windows separator",
    globMatchesText(R"(dir1\\*\**\\utils.cxx)", R"(dir1\anything*anything\utils.cxx)", '\\'));
  test("globToRegex: middle star escaped - no match",
    !globMatchesText("dir1/*\\**/utils.cxx", "dir1/dir2/utils.cxx"));
  test("globToRegex: middle star escaped - no match - Windows separator",
    !globMatchesText(R"(dir1\\*\**\\utils.cxx)", R"(dir1\dir2\utils.cxx)", '\\'));
  test("globToRegex: last star escaped",
    globMatchesText("dir1/**\\*/utils.cxx", "dir1/dir2*/utils.cxx"));
  test("globToRegex: last star escaped - Windows separator",
    globMatchesText(R"(dir1\\**\*\\utils.cxx)", R"(dir1\dir2*\utils.cxx)", '\\'));
  test("globToRegex: last star escaped - no match",
    !globMatchesText("dir1/**\\*/utils.cxx", "dir1/dir2/dir3*/utils.cxx"));
  test("globToRegex: last star escaped - no match - Windows separator",
    !globMatchesText(R"(dir1\\**\*\\utils.cxx)", "dir1/dir2/dir3*/utils.cxx", '\\'));
  test("globToRegex: star shouldn't cross directories",
    !globMatchesText("**/dir2*a.txt", "dir1/dir2/a.txt"));
  test("globToRegex: star shouldn't cross directories - Windows separator",
    !globMatchesText("**\\\\dir2*a.txt", "dir1\\dir2\\a.txt", '\\'));
  test("globToRegex: question mark shouldn't cross directories",
    !globMatchesText("**/dir2?a.txt", "dir1/dir2/a.txt"));
  test("globToRegex: question mark shouldn't cross directories - Windows separator",
    !globMatchesText("**\\\\dir2?a.txt", "dir1\\dir2\\a.txt", '\\'));
  test.expect<f3d::utils::glob_exception>("globToRegex: unclosed character class",
    [&]() { return f3d::utils::globToRegex("file[0-9.txt"); });
  test.expect<f3d::utils::glob_exception>(
    "globToRegex: unclosed alternation", [&]() { return f3d::utils::globToRegex("file.{txt,md"); });
  test.expect<f3d::utils::glob_exception>(
    "globToRegex: trailing escape", [&]() { return f3d::utils::globToRegex("file1.txt\\"); });

  //

  test("getEnv: standard", f3d::utils::getEnv("F3D_TEST_ENV_STANDARD") == "TestEnv");
  test("getEnv: empty", f3d::utils::getEnv("F3D_TEST_ENV_EMPTY") == std::nullopt);
  test("getEnv: extended", f3d::utils::getEnv("F3D_TEST_ENV_EXTENDED") == "Test\xC3\x8Bnv");
  test("getEnv: utf8", f3d::utils::getEnv("F3D_TEST_ENV_UTF8") == "\xF0\x9F\xA4\x94");

#if defined(_WIN32)
  test("getKnownFolder: windows roaming not empty",
    f3d::utils::getKnownFolder(f3d::utils::KnownFolder::ROAMINGAPPDATA) != std::nullopt);
  test("getKnownFolder: windows local not empty",
    f3d::utils::getKnownFolder(f3d::utils::KnownFolder::LOCALAPPDATA) != std::nullopt);
  test("getKnownFolder: windows pictures not empty",
    f3d::utils::getKnownFolder(f3d::utils::KnownFolder::PICTURES) != std::nullopt);
#else
  test("getKnownFolder: non-windows empty",
    f3d::utils::getKnownFolder(f3d::utils::KnownFolder::ROAMINGAPPDATA) == std::nullopt);
#endif

  return test.result();
}
