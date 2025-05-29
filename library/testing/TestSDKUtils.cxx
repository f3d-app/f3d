#include "PseudoUnitTest.h"

#include <utils.h>

#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

int TestSDKUtils(int argc, char* argv[])
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
                           bool supportDoubleStar = true) {
    const bool fullMatch = false;
    std::string regexPattern = f3d::utils::globToRegex(glob, fullMatch, supportDoubleStar);
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
  test("globToRegex: globstar simple", globMatchesText("**/file.txt", "dir/file.txt"));
  test("globToRegex: globstar deep", globMatchesText("**/file.txt", "dir1/dir2/dir3/file.txt"));
  test("globToRegex: glob disabled", globMatchesText("**/file.txt", "dir/file.txt", false));
  test("globToRegex: glob disabled - deep",
    globMatchesText("**/file.txt", "dir1/dir2/file.txt", false));
  test("globToRegex: glob disabled - no match",
    !globMatchesText("**/file.txt", "dir1/dir2/file1.txt", false));
  test("globToRegex: globstar prefix", globMatchesText("src/**/*.cpp", "src/module/file.cpp"));
  test("globToRegex: globstar middle",
    globMatchesText("src/**/include/*.h", "src/module/submodule/include/file.h"));
  test("globToRegex: empty glob", globMatchesText("", ""));
  test("globToRegex: single star", globMatchesText("*", "anything"));
  test("globToRegex: only special chars", globMatchesText("***??[a-z][0-9]", "abc5"));
  test("globToRegex: star shouldn't cross directories", !globMatchesText("*.txt", "dir/file.txt"));
  test("globToRegex: question mark shouldn't cross directories",
    !globMatchesText("file?a.txt", "file/a.txt"));
  test.expect<f3d::utils::glob_exception>("globToRegex: unclosed character class",
    [&]() { return f3d::utils::globToRegex("file[0-9.txt"); });
  test.expect<f3d::utils::glob_exception>(
    "globToRegex: unclosed alternation", [&]() { return f3d::utils::globToRegex("file.{txt,md"); });
  test.expect<f3d::utils::glob_exception>(
    "globToRegex: trailing escape", [&]() { return f3d::utils::globToRegex("file1.txt\\"); });

  return test.result();
}
