#ifndef Test_h
#define Test_h

#include <functional>
#include <iostream>
#include <sstream>

/** Helper to perform multiple checks within the same `ctest` test.
 * Checks are performed using the various overloads of `operator()`
 * and their results are logged and tracked so that the `result()` method
 * will return `EXIT_FAILURE` if any of them failed (and `EXIT_SUCCESS` otherwise).
 */
class PseudoUnitTest
{
public:
  /** test a boolean condition */
  void operator()(const std::string& label, bool condition)
  {
    this->record(condition, label);
  }

  /** test the execution of a function */
  void operator()(const std::string& label, std::function<void()> function)
  {
    this->testFunction<Dummy>(label, function);
  }

  /** test the execution of a function expecting a given exception */
  template<typename E>
  void expect(const std::string& label, std::function<void()> function)
  {
    this->testFunction<E>(label, function, true);
  }

  /** test the equality of two values with `==` */
  template<typename T1, typename T2>
  void operator()(const std::string& label, const T1& actual, const T2& expected)
  {
    const bool success = actual == expected;
    this->record(success, label, this->comparisonMessage(actual, expected, success ? "==" : "!="));
  }

  /** test the equality of two values with a comparator */
  template<typename T1, typename T2, typename F>
  void operator()(const std::string& label, const T1& actual, const T2& expected, F comparator)
  {
    const bool success = comparator(actual, expected);
    this->record(success, label, this->comparisonMessage(actual, expected, success ? "~=" : "!="));
  }

  int result()
  {
    const bool success = this->failCount == 0;
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
  }

private:
  size_t failCount = 0;
  size_t passCount = 0;

  /* execute `function` and:
   * - *fail* on normal termination when `expectException` is `true`
   * - *pass* on normal termination when `expectException` is `false`
   * - *pass* when an exception of type `E` is caught
   * - *fail* when anything else is caught
   */
  template<typename E>
  void testFunction(
    const std::string& label, std::function<void()> function, bool expectException = false)
  {
    try
    {
      function();
      if (expectException)
      {
        this->record(false, label, "did not throw");
      }
      else
      {
        this->record(true, label);
      }
    }
    catch (const E& e)
    {
      this->record(true, label, e.what());
    }
    catch (const char* msg)
    {
      this->record(false, label, msg);
    }
    catch (const std::string& msg)
    {
      this->record(false, label, msg);
    }
    catch (const std::exception& e)
    {
      this->record(false, label, std::string("unexpected exception (") + e.what() + ")");
    }
    catch (...)
    {
      this->record(false, label, "unexpected exception");
    }
  }

  void record(const bool success, const std::string& label, const std::string& message = "")
  {
    (success ? passCount : failCount)++;
    this->log(success, label, message);
  }

  template<typename T1, typename T2>
  std::string comparisonMessage(const T1& actual, const T2& expected, const std::string& comp)
  {
    const std::string actualStr = this->toString(actual);
    const std::string expectedStr = this->toString(expected);
    if (actualStr != expectedStr)
    {
      std::stringstream ss;
      ss << actualStr << " " << comp << " " << expectedStr;
      return ss.str();
    }
    else
    {
      return "";
    }
  }

  template<typename T>
  std::string toString(const T& value)
  {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }

protected:
  virtual void log(const bool success, const std::string& label, const std::string& message)
  {
    const std::string line = message.empty() ? label : (label + ": " + message);
    const std::string icon = success ? u8"\u2714" : u8"\u2718";
    (success ? std::cout : std::cerr) << icon << " " << line << std::endl;
  }

private:
  class Dummy : public std::exception
  {
  };
};

#endif
