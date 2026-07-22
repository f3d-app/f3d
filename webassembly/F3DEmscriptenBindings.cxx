#include <emscripten/bind.h>

#include <array>
#include <stdexcept>

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
  emscripten::enum_<f3d::options::domain_style>("OptionsDomainStyle")
    .value("RANGE", f3d::options::domain_style::RANGE)
    .value("ENUM", f3d::options::domain_style::ENUM)
    .value("INDEX", f3d::options::domain_style::INDEX);

  emscripten::enum_<f3d::options::option_type>("OptionType")
    .value("BOOL", f3d::options::option_type::BOOL)
    .value("INT", f3d::options::option_type::INT)
    .value("DOUBLE", f3d::options::option_type::DOUBLE)
    .value("RATIO", f3d::options::option_type::RATIO)
    .value("STRING", f3d::options::option_type::STRING)
    .value("PATH", f3d::options::option_type::PATH)
    .value("COLOR", f3d::options::option_type::COLOR)
    .value("DIRECTION", f3d::options::option_type::DIRECTION)
    .value("COLORMAP", f3d::options::option_type::COLORMAP)
    .value("TRANSFORM2D", f3d::options::option_type::TRANSFORM2D)
    .value("DOUBLE_VECTOR", f3d::options::option_type::DOUBLE_VECTOR)
    .value("INT_VECTOR", f3d::options::option_type::INT_VECTOR);

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
    .function(
      "getAsString",
      +[](f3d::options& o, const std::string& name) -> std::string { return o.getAsString(name); })
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
      "getType", +[](f3d::options& o, const std::string& name) -> f3d::options::option_type
      { return o.getType(name); })
    .function(
      "reset", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.reset(name); }, emscripten::return_value_policy::reference())
    .function(
      "removeValue", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.removeValue(name); }, emscripten::return_value_policy::reference())
    .function(
      "hasDomain",
      +[](const f3d::options& o, const std::string& name) -> bool { return o.hasDomain(name); })
    .function(
      "getDomainStyle",
      +[](const f3d::options& o, const std::string& name) -> f3d::options::domain_style
      { return o.getDomainStyle(name); })
    .function(
      "getEnumDomain", +[](const f3d::options& o, const std::string& name) -> emscripten::val
      { return containerToJSArray(o.getEnumDomain(name)); })
    .function(
      "getRangeDomain",
      +[](const f3d::options& o, const std::string& name) -> emscripten::val
      {
        f3d::options::DomainRange<f3d::option_variant_t> domain = o.getRangeDomain(name);
        // All range types are returned as JS numbers (doubles)
        auto toDouble = [](const f3d::option_variant_t& value) -> double
        {
          if (const int* intValue = std::get_if<int>(&value))
          {
            return static_cast<double>(*intValue);
          }
          if (const double* doubleValue = std::get_if<double>(&value))
          {
            return *doubleValue;
          }
          throw std::runtime_error("Range domain value cannot be represented as a JS number");
        };
        std::array<double, 3> values = { toDouble(domain.min), toDouble(domain.max),
          toDouble(domain.increment) };
        return containerToJSArray(values);
      })
    .function(
      "increase", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.increase(name); }, emscripten::return_value_policy::reference())
    .function(
      "decrease", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.decrease(name); }, emscripten::return_value_policy::reference())
    .function(
      "cycle", +[](f3d::options& o, const std::string& name) -> f3d::options&
      { return o.cycle(name); }, emscripten::return_value_policy::reference());

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
    .function(
      "addBuffer",
      +[](f3d::scene& scene, emscripten::val jsbuf) -> f3d::scene&
      {
        std::vector<unsigned char> data = emscripten::vecFromJSArray<unsigned char>(jsbuf);
        return scene.add(reinterpret_cast<std::byte*>(data.data()), data.size());
      },
      emscripten::return_value_policy::reference())
    .function("clear", &f3d::scene::clear, emscripten::return_value_policy::reference())
    .function(
      "getAddedFiles",
      +[](f3d::scene& scene) -> emscripten::val
      {
        std::vector<std::string> files;
        for (const std::filesystem::path& file : scene.getAddedFiles())
        {
          files.push_back(file.string());
        }
        return containerToJSArray(files);
      })
    .function("loadAnimationTime", &f3d::scene::loadAnimationTime,
      emscripten::return_value_policy::reference())
    .function(
      "animationTimeRange", +[](f3d::scene& scene) -> emscripten::val
      { return pairToJSArray(scene.animationTimeRange()); })
    .function(
      "getAnimationKeyFrames", +[](f3d::scene& scene) -> emscripten::val
      { return containerToJSArray(scene.getAnimationKeyFrames()); })
    .function("availableAnimations", &f3d::scene::availableAnimations)
    .function("getAnimationName", &f3d::scene::getAnimationName)
    .function(
      "getAnimationNames",
      +[](f3d::scene& scene) { return containerToJSArray(scene.getAnimationNames()); });

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
      "createFromBuffer",
      +[](emscripten::val jsbuf) -> f3d::image
      {
        std::vector<unsigned char> data = emscripten::vecFromJSArray<unsigned char>(jsbuf);
        return f3d::image(reinterpret_cast<std::byte*>(data.data()), data.size());
      })
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
    .function(
      "getWorldFromDisplay",
      +[](const f3d::window& win, emscripten::val jsArray) -> emscripten::val
      {
        return containerToJSArray(win.getWorldFromDisplay(
          { jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() }));
      })
    .function(
      "getDisplayFromWorld",
      +[](const f3d::window& win, emscripten::val jsArray) -> emscripten::val
      {
        return containerToJSArray(win.getDisplayFromWorld(
          { jsArray[0].as<float>(), jsArray[1].as<float>(), jsArray[2].as<float>() }));
      });

  // f3d::interactor
  emscripten::enum_<f3d::interactor::AnimationDirection>("InteractorAnimationDirection")
    .value("FORWARD", f3d::interactor::AnimationDirection::FORWARD)
    .value("BACKWARD", f3d::interactor::AnimationDirection::BACKWARD);

  // Not bound on purpose because usually used for external interactors:
  // trigger*
  // TODO:
  // - bindings
  emscripten::class_<f3d::interactor>("Interactor")
    .function(
      "initCommands", &f3d::interactor::initCommands, emscripten::return_value_policy::reference())
    .function(
      "addCommand",
      +[](f3d::interactor& interactor, const std::string& action,
         const emscripten::val& callback) -> f3d::interactor&
      {
        auto wrapCallback = [=](const std::vector<std::string>& args)
        { callback(containerToJSArray(args)); };
        return interactor.addCommand(action, wrapCallback);
      },
      emscripten::return_value_policy::reference())
    .function("removeCommand", &f3d::interactor::removeCommand,
      emscripten::return_value_policy::reference())
    .function(
      "getCommandActions", +[](const f3d::interactor& interactor) -> emscripten::val
      { return containerToJSArray(interactor.getCommandActions()); })
    .function(
      "triggerCommand",
      +[](f3d::interactor& interactor, const std::string& command, bool keepComments) -> bool
      { return interactor.triggerCommand(command, keepComments); })
    .function(
      "toggleAnimation",
      +[](f3d::interactor& interactor, emscripten::val direction) -> f3d::interactor&
      {
        return direction.isUndefined()
          ? interactor.toggleAnimation()
          : interactor.toggleAnimation(direction.as<f3d::interactor::AnimationDirection>());
      },
      emscripten::return_value_policy::reference())
    .function(
      "startAnimation",
      +[](f3d::interactor& interactor, emscripten::val direction) -> f3d::interactor&
      {
        return direction.isUndefined()
          ? interactor.startAnimation()
          : interactor.startAnimation(direction.as<f3d::interactor::AnimationDirection>());
      },
      emscripten::return_value_policy::reference())
    .function("stopAnimation", &f3d::interactor::stopAnimation,
      emscripten::return_value_policy::reference())
    .function("isPlayingAnimation", &f3d::interactor::isPlayingAnimation)
    .function("getAnimationDirection", &f3d::interactor::getAnimationDirection)
    .function("enableCameraMovement", &f3d::interactor::enableCameraMovement,
      emscripten::return_value_policy::reference())
    .function("disableCameraMovement", &f3d::interactor::disableCameraMovement,
      emscripten::return_value_policy::reference())
    .function(
      "start", +[](f3d::interactor& interactor) -> f3d::interactor& { return interactor.start(); },
      emscripten::return_value_policy::reference())
    .function("stop", &f3d::interactor::stop, emscripten::return_value_policy::reference())
    .function("requestRender", &f3d::interactor::requestRender,
      emscripten::return_value_policy::reference())
    .function(
      "requestStop", &f3d::interactor::requestStop, emscripten::return_value_policy::reference())
    .function(
      "triggerNotification",
      +[](f3d::interactor& interactor, std::string desc, std::string value, double duration)
      { interactor.triggerNotification(desc, value, duration); })
    .function(
      "setNotificationCallback",
      +[](f3d::interactor& interactor, const emscripten::val& callback)
      {
        if (callback.isUndefined() || callback.isNull())
        {
          interactor.setNotificationCallback(nullptr);
          return;
        }

        auto cb = [=](const std::string& desc, const std::string& value, const std::string& bind,
                    double duration) -> bool
        { return callback(desc, value, bind, duration).as<bool>(); };

        interactor.setNotificationCallback(cb);
      });

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

  emscripten::class_<f3d::engine::state>("EngineState")
    .class_function(
      "fromString",
      +[](const std::string& content) { return f3d::engine::state::fromString(content); })
    .class_function(
      "fromFile", +[](const std::string& path) { return f3d::engine::state::fromFile(path); })
    .function("toString", &f3d::engine::state::toString)
    .function(
      "toFile",
      +[](const f3d::engine::state& state, const std::string& path) { state.toFile(path); });

  emscripten::class_<f3d::engine>("Engine")
    .class_function(
      "create", +[](std::string canvas) { return f3d::engine::createWasm(canvas); },
      emscripten::return_value_policy::take_ownership())
    .class_function(
      "create", +[]() { return f3d::engine::createWasm(); },
      emscripten::return_value_policy::take_ownership())
    .function(
      "setCachePath", +[](f3d::engine& engine, const std::string& path) -> f3d::engine&
      { return engine.setCachePath(path); }, emscripten::return_value_policy::reference())
    .function("setOptions",
      static_cast<f3d::engine& (f3d::engine::*)(const f3d::options&)>(&f3d::engine::setOptions),
      emscripten::return_value_policy::reference())
    .function("getOptions", &f3d::engine::getOptions, emscripten::return_value_policy::reference())
    .function("getWindow", &f3d::engine::getWindow, emscripten::return_value_policy::reference())
    .function("getScene", &f3d::engine::getScene, emscripten::return_value_policy::reference())
    .function(
      "dump", +[](f3d::engine& engine) { return engine.dump(); })
    .function(
      "load", +[](f3d::engine& engine, const f3d::engine::state& state) -> f3d::engine&
      { return engine.load(state); }, emscripten::return_value_policy::reference())
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

  emscripten::class_<f3d::log>("Log")
    .class_function("setVerboseLevel", f3d::log::setVerboseLevel)
    .class_function("getVerboseLevel", f3d::log::getVerboseLevel)
    .class_function("setUseColoring", f3d::log::setUseColoring)
    .class_function(
      "print",
      +[](f3d::log::VerboseLevel level, const std::string& message)
      { f3d::log::print(level, message); })
    .class_function(
      "forward",
      +[](const emscripten::val& callback)
      {
        f3d::log::forward(
          [=](f3d::log::VerboseLevel level, const std::string& txt) { callback(level, txt); });
      });
}
