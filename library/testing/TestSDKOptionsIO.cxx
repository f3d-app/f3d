#include <export.h>
#include <options.h>

#include "PseudoUnitTest.h"

#include <iostream>

class ParsingTest : public PseudoUnitTest
{
public:
  template<typename T>
  void parse(const std::string& label, const std::string& input, const T& expected)
  {
    PseudoUnitTest::operator()(label + " `" + input + "`", [&]() {
      const T actual = f3d::options::parse<T>(input);
      if (actual != expected)
      {
        throw this->comparisonMessage(actual, expected, "!=");
      }
    });
  }

  template<typename T, typename E>
  void parse_expect(const std::string& label, const std::string& input)
  {
    PseudoUnitTest::expect<E>(
      label + " `" + input + "`", [&]() { std::ignore = f3d::options::parse<T>(input); });
  }
};

int TestSDKOptionsIO(int argc, char* argv[])
{
  using parsing_exception = f3d::options::parsing_exception;
  ParsingTest test;

  test.parse<bool>("bool", "true", true);
  test.parse<bool>("bool", "false", false);
  test.parse<bool>("bool", "yes", true);
  test.parse<bool>("bool", "Yes", true);
  test.parse<bool>("bool", "no", false);
  test.parse<bool>("bool", "1", true);
  test.parse<bool>("bool", "0", false);
  test.parse_expect<bool, parsing_exception>("invalid bool", "foo");

  test.parse<int>("int", "123", 123);
  test.parse<int>("int", "-123", -123);
  test.parse<int>("int", "+123", +123);
  test.parse_expect<int, parsing_exception>("invalid int", "1.2");
  test.parse_expect<int, parsing_exception>("invalid int", "abc");
  test.parse_expect<int, parsing_exception>("invalid int",
    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");

  test.parse<double>("double", "123", 123.0);
  test.parse<double>("double", "-123.45", -123.45);
  test.parse<double>("double", "+1e-3", 0.001);
  test.parse_expect<double, parsing_exception>("invalid double", "1.2.3");
  test.parse_expect<double, parsing_exception>("invalid double", "abc");

  test.parse<std::string>("std::string", "foobar", "foobar");
  test.parse<std::string>("std::string", "  foobar   ", "foobar");

  test.parse<f3d::ratio_t>("ratio_t", "0.1234", 0.1234);
  test.parse<f3d::ratio_t>("ratio_t", "12.34%", 0.1234);
  test.parse_expect<f3d::ratio_t, parsing_exception>("invalid ratio_t", "12.34&");
  test.parse<f3d::ratio_t>("ratio_t", "-2/-3.5", 2.0 / 3.5);
  test.parse_expect<f3d::ratio_t, parsing_exception>("invalid ratio_t", "1/2/3");

  test.parse<std::vector<int>>("std::vector<int>", "1, 2, 3", { 1, 2, 3 });

  test.parse<std::vector<double>>("std::vector<double>", "  0.1,  0.2 , 0.3 ", { 0.1, 0.2, 0.3 });

  test.parse<std::vector<std::string>>(
    "std::vector<std::string>", "  foo, bar ,   baz ", { "foo", "bar", "baz" });

  return test.result();
}
