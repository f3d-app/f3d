#include <emscripten/bind.h>

#include "camera.h"
#include "engine.h"
#include "interactor.h"
#include "options.h"
#include "scene.h"
#include "window.h"

// This is needed to avoid compilation issues because the destructors are protected
namespace emscripten::internal
{
template<>
void raw_destructor(f3d::scene* ptr)
{
}

template<>
void raw_destructor(f3d::window* ptr)
{
}

template<>
void raw_destructor(f3d::interactor* ptr)
{
}

template<>
void raw_destructor(f3d::camera* ptr)
{
}
}

template<typename T>
emscripten::val containerToJSArray(const T& container)
{
  emscripten::val jsArray = emscripten::val::array();
  for (const auto& elem : container)
  {
    jsArray.call<void>("push", elem);
  }
  return jsArray;
}

template<typename U, typename V>
emscripten::val pairToJSArray(const std::pair<U, V>& p)
{
  emscripten::val jsArray = emscripten::val::array();
  jsArray.call<void>("push", p.first);
  jsArray.call<void>("push", p.second);
  return jsArray;
}

EMSCRIPTEN_BINDINGS(f3d)
{
  // f3d::options
  emscripten::class_<f3d::options>("Options")
    .constructor<>()
    .function(
      "get",
      +[](f3d::options& options, const std::string& name) -> emscripten::val
      {
        try
        {
          f3d::option_variant_t value = options.get(name);
          if (std::holds_alternative<bool>(value))
          {
            return emscripten::val(std::get<bool>(value));
          }
          if (std::holds_alternative<int>(value))
          {
            return emscripten::val(std::get<int>(value));
          }
          if (std::holds_alternative<double>(value))
          {
            return emscripten::val(std::get<double>(value));
          }
          if (std::holds_alternative<std::string>(value))
          {
            return emscripten::val(std::get<std::string>(value));
          }
          if (std::holds_alternative<std::vector<double>>(value))
          {
            return containerToJSArray(std::get<std::vector<double>>(value));
          }
          if (std::holds_alternative<std::vector<int>>(value))
          {
            return containerToJSArray(std::get<std::vector<int>>(value));
          }
        }
        catch (f3d::exception)
        {
          return emscripten::val::undefined();
        }

        return emscripten::val::undefined();
      })
    .function(
      "setAsString",
      +[](f3d::options& o, const std::string& name, const std::string& value) -> f3d::options&
      { return o.setAsString(name, value); }, emscripten::return_value_policy::reference())
    .function("getAsString", &f3d::options::getAsString)
    .function(
      "toggle", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.toggle(name); }, emscripten::return_value_policy::reference())
    .function(
      "isSame", +[](f3d::options& o, f3d::options& other, const std::string& name) -> bool
      { return o.isSame(other, name); })
    .function(
      "hasValue",
      +[](f3d::options& o, const std::string& name) -> bool { return o.hasValue(name); })
    .function(
      "copy", +[](f3d::options& o, f3d::options& other, const std::string& name) -> f3d::options&
      { return o.copy(other, name); }, emscripten::return_value_policy::reference())
    .class_function(
      "getAllNames",
      +[]() -> emscripten::val { return containerToJSArray(f3d::options::getAllNames()); })
    .function(
      "getNames",
      +[](f3d::options& o) -> emscripten::val { return containerToJSArray(o.getNames()); })
    .function(
      "getClosestOption", +[](f3d::options& o, const std::string& name) -> emscripten::val
      { return pairToJSArray(o.getClosestOption(name)); })
    .function(
      "isOptional",
      +[](f3d::options& o, const std::string& name) -> bool { return o.isOptional(name); })
    .function(
      "reset", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.reset(name); }, emscripten::return_value_policy::reference())
    .function(
      "removeValue", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.removeValue(name); }, emscripten::return_value_policy::reference());

  // f3d::scene
  // TODO:
  // - add lights support
  // - add f3d::mesh_t support
  emscripten::class_<f3d::scene>("Scene")
    .function(
      "supports",
      +[](f3d::scene& scene, const std::string& path) -> bool { return scene.supports(path); })
    .function(
      "add",
      +[](f3d::scene& scene, emscripten::val arg) -> f3d::scene&
      {
        if (arg.isArray())
        {
          return scene.add(emscripten::vecFromJSArray<std::string>(arg));
        }
        else
        {
          return scene.add(arg.as<std::string>());
        }
      },
      emscripten::return_value_policy::reference())
    .function("clear", &f3d::scene::clear, emscripten::return_value_policy::reference())
    .function("loadAnimationTime", &f3d::scene::loadAnimationTime,
      emscripten::return_value_policy::reference())
    .function(
      "animationTimeRange",
      +[](f3d::scene& o) -> emscripten::val { return pairToJSArray(o.animationTimeRange()); })
    .function("availableAnimations", &f3d::scene::availableAnimations);

  // f3d::image
  emscripten::enum_<f3d::image::SaveFormat>("ImageSaveFormat")
    .value("PNG", f3d::image::SaveFormat::PNG)
    .value("JPG", f3d::image::SaveFormat::JPG)
    .value("TIF", f3d::image::SaveFormat::TIF)
    .value("BMP", f3d::image::SaveFormat::BMP);

  emscripten::enum_<f3d::image::ChannelType>("ImageChannelType")
    .value("BYTE", f3d::image::ChannelType::BYTE)
    .value("SHORT", f3d::image::ChannelType::SHORT)
    .value("FLOAT", f3d::image::ChannelType::FLOAT);

  emscripten::class_<f3d::image>("Image")
    .constructor<>()
    .constructor<const std::string&>()
    .constructor<unsigned int, unsigned int, unsigned int, f3d::image::ChannelType>()
    .function("equals", &f3d::image::operator==)
    .function(
      "getNormalizedPixel", +[](const f3d::image& img, int x, int y) -> emscripten::val
      { return containerToJSArray(img.getNormalizedPixel({ x, y })); })
    .class_function(
      "getSupportedFormats",
      +[]() -> emscripten::val { return containerToJSArray(f3d::image::getSupportedFormats()); })
    .property("width", &f3d::image::getWidth)
    .property("height", &f3d::image::getHeight)
    .property("channelCount", &f3d::image::getChannelCount)
    .property("channelType", &f3d::image::getChannelType)
    .property("channelTypeSize", &f3d::image::getChannelTypeSize)
    .function(
      "setContent",
      +[](f3d::image& img, emscripten::val jsbuf) -> f3d::image&
      {
        std::vector<uint8_t> data = emscripten::vecFromJSArray<uint8_t>(jsbuf);
        size_t expected = static_cast<size_t>(img.getWidth()) * img.getHeight() *
          img.getChannelCount() * img.getChannelTypeSize();
        if (data.size() != expected)
        {
          throw std::runtime_error("Buffer size does not match image size");
        }
        img.setContent(data.data());
        return img;
      },
      emscripten::allow_raw_pointers())
    .function(
      "getContent",
      +[](const f3d::image& img) -> emscripten::val
      {
        size_t totalSize =
          img.getWidth() * img.getHeight() * img.getChannelCount() * img.getChannelTypeSize();
        return emscripten::val(
          emscripten::typed_memory_view(totalSize, static_cast<const uint8_t*>(img.getContent())));
      },
      emscripten::allow_raw_pointers())
    .function("compare", &f3d::image::compare)
    .function(
      "save",
      +[](const f3d::image& img, const std::string& path,
         f3d::image::SaveFormat format) -> const f3d::image& { return img.save(path, format); },
      emscripten::allow_raw_pointers())
    .function(
      "saveBuffer",
      +[](const f3d::image& img, f3d::image::SaveFormat format) -> emscripten::val
      {
        std::vector<uint8_t> buffer = img.saveBuffer(format);
        return emscripten::val(emscripten::typed_memory_view(buffer.size(), buffer.data()));
      })
    .function("toTerminalText",
      static_cast<std::string (f3d::image::*)() const>(&f3d::image::toTerminalText))
    .function("setMetadata", &f3d::image::setMetadata, emscripten::allow_raw_pointers())
    .function("getMetadata", &f3d::image::getMetadata)
    .function(
      "allMetadata", +[](const f3d::image& img) -> emscripten::val
      { return containerToJSArray(img.allMetadata()); });

  // f3d::camera
  // TODO:
  // - camera state
  emscripten::class_<f3d::camera>("Camera")
    .property(
      "position", +[](const f3d::camera& cam) -> emscripten::val
      { return containerToJSArray(cam.getPosition()); },
      +[](f3d::camera& cam, emscripten::val jsArray) {
        cam.setPosition({ jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() });
      })
    .property(
      "focalPoint", +[](const f3d::camera& cam) -> emscripten::val
      { return containerToJSArray(cam.getFocalPoint()); },
      +[](f3d::camera& cam, emscripten::val jsArray) {
        cam.setFocalPoint(
          { jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() });
      })
    .property(
      "viewUp", +[](const f3d::camera& cam) -> emscripten::val
      { return containerToJSArray(cam.getViewUp()); },
      +[](f3d::camera& cam, emscripten::val jsArray) {
        cam.setViewUp({ jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() });
      })
    .property("viewAngle",
      static_cast<f3d::angle_deg_t (f3d::camera::*)() const>(&f3d::camera::getViewAngle),
      &f3d::camera::setViewAngle)
    .function("dolly", &f3d::camera::dolly, emscripten::return_value_policy::reference())
    .function("pan", &f3d::camera::pan, emscripten::return_value_policy::reference())
    .function("zoom", &f3d::camera::zoom, emscripten::return_value_policy::reference())
    .function("roll", &f3d::camera::roll, emscripten::return_value_policy::reference())
    .function("azimuth", &f3d::camera::azimuth, emscripten::return_value_policy::reference())
    .function("yaw", &f3d::camera::yaw, emscripten::return_value_policy::reference())
    .function("elevation", &f3d::camera::elevation, emscripten::return_value_policy::reference())
    .function("pitch", &f3d::camera::pitch, emscripten::return_value_policy::reference())
    .function("setCurrentAsDefault", &f3d::camera::setCurrentAsDefault,
      emscripten::return_value_policy::reference())
    .function(
      "resetToDefault", &f3d::camera::resetToDefault, emscripten::return_value_policy::reference())
    .function(
      "resetToBounds", &f3d::camera::resetToBounds, emscripten::return_value_policy::reference());

  // f3d::window
  // Not bound on purpose because these functions make no sense on the web:
  // getType, isOffscreen, setPosition, setIcon, setWindowName
  emscripten::class_<f3d::window>("Window")
    .function("getCamera", &f3d::window::getCamera, emscripten::return_value_policy::reference())
    .function("render", &f3d::window::render)
    .function("renderToImage", &f3d::window::renderToImage)
    .function("setSize", &f3d::window::setSize, emscripten::return_value_policy::reference())
    .property("width", &f3d::window::getWidth)
    .property("height", &f3d::window::getHeight)
    .function("getWorldFromDisplay", +[](const f3d::window& win, emscripten::val jsArray) -> emscripten::val
      { return containerToJSArray(win.getWorldFromDisplay({ jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() })); })
    .function("getDisplayFromWorld", +[](const f3d::window& win, emscripten::val jsArray) -> emscripten::val
      { return containerToJSArray(win.getDisplayFromWorld({ jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() })); });

  // f3d::interactor
  // Not bound on purpose because usually used for external interactors:
  // trigger*
  // TODO:
  // - bindings
  emscripten::class_<f3d::interactor>("Interactor")
    .function(
      "initCommands", &f3d::interactor::initCommands, emscripten::return_value_policy::reference())
    .function("addCommand",
      +[](f3d::interactor& interactor, const std::string& action, const emscripten::val& callback) -> f3d::interactor&
      {
        auto wrapCallback = [=](const std::vector<std::string>& args)
        {
          callback(containerToJSArray(args));
        };
        return interactor.addCommand(action, wrapCallback);
      },
      emscripten::return_value_policy::reference())
    .function("removeCommand", &f3d::interactor::removeCommand,
      emscripten::return_value_policy::reference())
    .function("getCommandActions", +[](const f3d::interactor& interactor) -> emscripten::val
      { return containerToJSArray(interactor.getCommandActions()); })
    .function("triggerCommand", +[](f3d::interactor& interactor, const std::string& command, bool keepComments) -> bool
      { return interactor.triggerCommand(command, keepComments); })
    .function("toggleAnimation", &f3d::interactor::toggleAnimation,
      emscripten::return_value_policy::reference())
    .function("startAnimation", &f3d::interactor::startAnimation,
      emscripten::return_value_policy::reference())
    .function("stopAnimation", &f3d::interactor::stopAnimation,
      emscripten::return_value_policy::reference())
    .function("isPlayingAnimation", &f3d::interactor::isPlayingAnimation)
    .function("enableCameraMovement", &f3d::interactor::enableCameraMovement,
      emscripten::return_value_policy::reference())
    .function("disableCameraMovement", &f3d::interactor::disableCameraMovement,
      emscripten::return_value_policy::reference())
    .function(
      "start", +[](f3d::interactor& interactor) -> f3d::interactor& { return interactor.start(); },
      emscripten::return_value_policy::reference())
    .function("stop", &f3d::interactor::stop, emscripten::return_value_policy::reference())
    .function("requestRender", &f3d::interactor::requestRender,
      emscripten::return_value_policy::reference());

  // f3d::engine
  // Not bound on purpose because only one engine is supported:
  // create*, getRenderingBackendList
  emscripten::class_<f3d::engine::libInformation>("EngineLibInformation")
    .property("version", &f3d::engine::libInformation::Version)
    .property("versionFull", &f3d::engine::libInformation::VersionFull)
    .property("buildDate", &f3d::engine::libInformation::BuildDate)
    .property("buildSystem", &f3d::engine::libInformation::BuildSystem)
    .property("compiler", &f3d::engine::libInformation::Compiler)
    .property(
      "modules",
      +[](const f3d::engine::libInformation& libInfo) -> emscripten::val
      {
        emscripten::val jsArray = emscripten::val::array();
        for (const auto& [module, enabled] : libInfo.Modules)
        {
          emscripten::val pair = emscripten::val::array();
          pair.call<void>("push", module);
          pair.call<void>("push", enabled);
          jsArray.call<void>("push", pair);
        }
        return jsArray;
      })
    .property("vtkVersion", &f3d::engine::libInformation::VTKVersion)
    .property(
      "copyrights", +[](const f3d::engine::libInformation& libInfo) -> emscripten::val
      { return containerToJSArray(libInfo.Copyrights); })
    .property("license", &f3d::engine::libInformation::License);

  emscripten::class_<f3d::engine::readerInformation>("EngineReaderInformation")
    .property("name", &f3d::engine::readerInformation::Name)
    .property("description", &f3d::engine::readerInformation::Description)
    .property(
      "extensions", +[](const f3d::engine::readerInformation& readerInfo) -> emscripten::val
      { return containerToJSArray(readerInfo.Extensions); })
    .property(
      "mimeTypes", +[](const f3d::engine::readerInformation& readerInfo) -> emscripten::val
      { return containerToJSArray(readerInfo.MimeTypes); })
    .property("pluginName", &f3d::engine::readerInformation::PluginName)
    .property("hasSceneReader", &f3d::engine::readerInformation::HasSceneReader)
    .property("hasGeometryReader", &f3d::engine::readerInformation::HasGeometryReader);

  emscripten::class_<f3d::engine>("Engine")
    .class_function(
      "create", +[]() { return f3d::engine::create(); },
      emscripten::return_value_policy::take_ownership())
    .function(
      "setCachePath", &f3d::engine::setCachePath, emscripten::return_value_policy::reference())
    .function("setOptions",
      static_cast<f3d::engine& (f3d::engine::*)(const f3d::options&)>(&f3d::engine::setOptions),
      emscripten::return_value_policy::reference())
    .function("getOptions", &f3d::engine::getOptions, emscripten::return_value_policy::reference())
    .function("getWindow", &f3d::engine::getWindow, emscripten::return_value_policy::reference())
    .function("getScene", &f3d::engine::getScene, emscripten::return_value_policy::reference())
    .function(
      "getInteractor", &f3d::engine::getInteractor, emscripten::return_value_policy::reference())
    .class_function("autoloadPlugins", &f3d::engine::autoloadPlugins)
    .class_function(
      "getAllReaderOptionNames", +[]() -> emscripten::val
      { return containerToJSArray(f3d::engine::getAllReaderOptionNames()); })
    .class_function("setReaderOption", &f3d::engine::setReaderOption)
    .class_function("getLibInfo", &f3d::engine::getLibInfo)
    .class_function(
      "getReadersInfo",
      +[]() -> emscripten::val { return containerToJSArray(f3d::engine::getReadersInfo()); });

  // f3d::log
  emscripten::enum_<f3d::log::VerboseLevel>("LogVerboseLevel")
    .value("DEBUG", f3d::log::VerboseLevel::DEBUG)
    .value("INFO", f3d::log::VerboseLevel::INFO)
    .value("WARN", f3d::log::VerboseLevel::WARN)
    .value("ERROR", f3d::log::VerboseLevel::ERROR)
    .value("QUIET", f3d::log::VerboseLevel::QUIET);

  emscripten::class_<f3d::log>("Log").class_function("setVerboseLevel", f3d::log::setVerboseLevel);
}
