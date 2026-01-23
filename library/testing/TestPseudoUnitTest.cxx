#include <export.h>
#include <options.h>

#include "PseudoUnitTest.h"

#include <iostream>

int TestPseudoUnitTest([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest metatest;

  class InnerTest : public PseudoUnitTest
  {
  protected:
    void log(const bool success, const std::string& label, const std::string& message) override
    {
      (success ? std::cout : std::cerr) << "  ";
      PseudoUnitTest::log(success, label, message);
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
    test("pass", 1.0, 1.0);
    metatest("pass with 1.0 == 1.0", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test("fail", 1.0, 1.0 + std::numeric_limits<double>::epsilon());
    metatest("fail with 1.0 == 1.0 + epsilon", test.result() == EXIT_FAILURE);
  }

  {
    InnerTest test;
    test.fuzzyCompare("pass", 1, 1);
    metatest("pass with 1 almost 1", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test.fuzzyCompare("pass", 1.0, 1.0);
    metatest("pass with 1 almost 1.0", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test.fuzzyCompare("pass", 1.0, 1.0 + std::numeric_limits<double>::epsilon());
    metatest("pass with 1 almost 1 + epsilon", test.result() == EXIT_SUCCESS);
  }
  {
    InnerTest test;
    test.fuzzyCompare("fail", 1.0, 2.0001);
    metatest("fail with 1 almost 2.0001", test.result() == EXIT_FAILURE);
  }

  return metatest.result();
}
