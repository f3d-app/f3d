#include "PseudoUnitTest.h"

#include <utils.h>

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

  return test.result();
}
