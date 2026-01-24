#ifndef PseudoUnitTest_h
#define PseudoUnitTest_h

#include "options.h"
#include "types.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace
{
template<typename T>
struct is_container : std::false_type
{
};

template<>
struct is_container<f3d::point3_t> : std::true_type
{
};

template<>
struct is_container<f3d::vector3_t> : std::true_type
{
};

template<typename T>
struct is_container<std::vector<T>> : std::true_type
{
};

template<typename T>
std::string toString(const T& value)
{
  std::stringstream ss;

  if constexpr (is_container<T>::value)
  {
    size_t i = 0;
    for (const auto& item : value)
    {
      ss << (i++ ? ", " : "{ ") << ::toString(item);
    }
    ss << " }";
  }
  else
  {
    ss << value;
  }
  return ss.str();
}
}

template<typename T>
class approx
{
public:
  approx(const T value, double tol = 128 * std::numeric_limits<double>::epsilon())
    : value(value)
    , tol(tol)
  {
  }
  bool operator==(const T& rhs) const
  {
    auto fuzzyComp = [this](double a, double b) { return std::fabs(a - b) < this->tol; };

    if constexpr (is_container<T>::value)
    {
      return std::equal(this->value.begin(), this->value.end(), rhs.begin(), fuzzyComp);
    }
    else
    {
      return fuzzyComp(this->value, rhs);
    }
  }

  const T value;
  const double tol;
};

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
  template<typename F>
  void operator()(const std::string& label, F function)
  {
    this->testFunction<Dummy>(label, function);
  }

  /** test the execution of a function expecting a given exception */
  template<typename E, typename F>
  void expect(const std::string& label, F function)
  {
    this->testFunction<E>(label, function);
  }

  /** test the equality of two values */
  template<typename T>
  void operator()(const std::string& label, const T& actual, const T& expected)
  {
    const bool success = actual == expected;
    this->record(success, label, this->comparisonMessage(actual, expected, success ? "==" : "!="));
  }

  /** test the equality of two values with fuzzy comparison */
  template<typename T>
  void operator()(const std::string& label, const approx<T>& actual_approx, const T& expected)
  {
    const bool success = actual_approx == expected;
    this->record(success, label,
      this->comparisonMessage(actual_approx.value, expected, success ? "~=" : "!="));
  }

  /** test the equality of two values with fuzzy comparison */
  template<typename T>
  void operator()(const std::string& label, const T& actual, const approx<T>& expected_approx)
  {
    const bool success = expected_approx == actual;
    this->record(success, label,
      this->comparisonMessage(actual, expected_approx.value, success ? "~=" : "!="));
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
   * - *pass* on normal termination when `E` is `Dummy`
   * - *fail* on normal termination when `E` is any other exception
   * - *pass* when an exception of type `E` is caught
   * - *fail* when anything else is caught
   */
  template<typename E, typename F>
  void testFunction(const std::string& label, F function)
  {
    try
    {
      function();
      if (std::is_same<E, Dummy>::value)
      {
        this->record(true, label);
      }
      else
      {
        this->record(false, label, "did not throw");
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

protected:
  template<typename T>
  std::string comparisonMessage(const T& actual, const T& expected, const std::string& comp)
  {
    const std::string actualStr = ::toString(actual);
    const std::string expectedStr = ::toString(expected);
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

  virtual void log(const bool success, const std::string& label, const std::string& message)
  {
    const std::string line = message.empty() ? label : (label + ": " + message);
    const std::string icon = success ? u8"\u2714" : u8"\u2718";
    (success ? std::cout : std::cerr) << icon << " " << line << "\n";
  }

private:
  class Dummy : public std::exception
  {
  };
};

#endif
