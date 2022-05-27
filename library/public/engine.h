/**
 * @class   engine
 * @brief   Class used to create instance of other classes
 *
 * TODO improve doc
 */

#ifndef f3d_engine_h
#define f3d_engine_h

#include "export.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace f3d
{
class options;
class window;
class loader;
class interactor;
class F3D_EXPORT engine
{
public:
  class exception : public std::runtime_error
  {
  public:
    exception(const std::string& what = "")
      : std::runtime_error(what)
    {
    }
  };

  //======== Engine Flags =============
  // engine::CREATE_WINDOW: Create a window to render into.
  // engine::CREATE_INTERACTOR: Create an interactor to interact with.
  // engine::WINDOW_OFFSCREEN: Create an offscreen window to render into, need CREATE_WINDOW.
  using flags_t = uint32_t;
  enum Flags : flags_t
  {
    FLAGS_NONE = 0,             // 0000
    CREATE_WINDOW = 1 << 0,     // 0001
    CREATE_INTERACTOR = 1 << 1, // 0010
    WINDOW_OFFSCREEN = 1 << 2   // 0100
  };

  explicit engine(const flags_t& flags);
  ~engine();

  // Engine provide a default options
  // use this setter to use other options
  // copy options into engine
  void setOptions(const options& opt);

  // Engine provide a default options
  // use this setter to use other options
  // move options into engine
  void setOptions(options&& opt);

  options& getOptions();
  window& getWindow();
  loader& getLoader();
  interactor& getInteractor();

  // Get a map containing info about the libf3d
  static std::map<std::string, std::string> getLibInfo();

  struct readerInformation
  {
    std::string name;
    std::string description;
    std::vector<std::string> extensions;
    std::vector<std::string> mimetypes;
  };

  // Get a vector containing info about the supported readers
  static std::vector<readerInformation> getReadersInfo();

private:
  class internals;
  internals* Internals;
  engine(const engine& opt) = delete;
  engine& operator=(const engine& opt) = delete;
};
}

#endif
