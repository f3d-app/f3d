#include <export.h>
#include <options.h>

#include "test.h"
#include <iostream>

int TestTestHelper(int argc, char* argv[])
{
  Test metatest;

  class InnerTest : public Test
  {
  protected:
    void log(const bool success, const std::string& label, const std::string& message) override
    {
      (success ? std::cout : std::cerr) << "  ";
      Test::log(success, label, message);
    }
  };

  {
    InnerTest test;
    test("pass", [&]() {
      /* no issue here */
    });
    metatest("pass on ok function", test.result() == EXIT_SUCCESS);
  }

  {
    InnerTest test;
    test("fail", [&]() { throw "test failure"; });
    metatest("fail with throw char*", test.result() == EXIT_FAILURE);
  }
  {
    InnerTest test;
    test("fail", [&]() { throw std::string("test failure: ") + "foobar"; });
    metatest("fail with throw string", test.result() == EXIT_FAILURE);
  }
  {
    InnerTest test;
    test("fail", [&]() { throw 12.34; });
    metatest("fail with throw whatever", test.result() == EXIT_FAILURE);
  }

  {
    InnerTest test;
    test("fail", [&]() { throw std::domain_error("blah"); });
    metatest("fail on exception", test.result() == EXIT_FAILURE);
  }
  {
    InnerTest test;
    test.expect<std::range_error>("pass", [&]() { throw std::range_error("hello"); });
    metatest("pass on expected exception", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test.expect<std::range_error>("fail", [&]() { throw std::domain_error("blah"); });
    metatest("fail on unexpected exception", test.result() == EXIT_FAILURE);
  }
  {
    InnerTest test;
    test.expect<std::range_error>("fail", [&]() { /* doesn't throw */ });
    metatest("fail on missing exception", test.result() == EXIT_FAILURE);
  }

  {
    InnerTest test;
    test("pass", 1, 1);
    metatest("pass with 1 == 1", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test("pass", 1, 1.0);
    metatest("pass with 1 == 1.0", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test("fail", 1, 1.0001);
    metatest("fail with 1 == 1.0001", test.result() == EXIT_FAILURE);
  }

  const auto kindaEq = [](double a, double b) { return std::abs(b - a) < 1e-3; };
  {
    InnerTest test;
    test("pass", 1, 1, kindaEq);
    metatest("pass with 1 almost 1", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test("pass", 1, 1.0, kindaEq);
    metatest("pass with 1 almost 1.0", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test("pass", 1, 1.0001, kindaEq);
    metatest("pass with 1 almost 1.0001", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test("fail", 1, 2.0001, kindaEq);
    metatest("fail with 1 almost 2.0001", test.result() == EXIT_FAILURE);
  }

  return metatest.result();
}
