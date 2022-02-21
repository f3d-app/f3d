#ifndef f3d_options_h
#define f3d_options_h

#include <memory>
#include <string>

// TODO documentation

namespace f3d
{
class options
{
public:
  options();
  ~options();
  options(const options& opt);

  options& operator=(const options& opt);

  template<typename T>
  void set(const std::string& name, const T& value);

  template<typename T>
  void get(const std::string& name, T& value) const;

  template<typename T>
  T get(const std::string& name) const;

  static void initialize(options& options);

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif
