#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "camera.h"
#include "engine.h"
#include "image.h"
#include "interactor.h"
#include "loader.h"
#include "options.h"
#include "types.h"
#include "utils.h"
#include "window.h"

namespace py = pybind11;

template<typename T, size_t S>
bool load_array(const py::handle& src, bool convert, std::array<T, S>& value)
{
  if (!py::isinstance<py::sequence>(src))
  {
    return false;
  }
  const py::sequence l = py::reinterpret_borrow<py::sequence>(src);
  if (l.size() != S)
  {
    return false;
  }

  size_t i = 0;
  for (auto it : l)
  {
    value[i++] = py::cast<T>(it);
  }
  return true;
}

template<>
class py::detail::type_caster<f3d::point3_t>
{
public:
  bool load(handle src, bool convert) { return load_array(src, convert, value); }

  static handle cast(const f3d::point3_t& src, return_value_policy, handle /* parent */)
  {
    return Py_BuildValue("ddd", src[0], src[1], src[2]);
  }

  PYBIND11_TYPE_CASTER(f3d::point3_t, const_name("f3d.point3_t"));
};

template<>
class py::detail::type_caster<f3d::vector3_t>
{
public:
  bool load(handle src, bool convert) { return load_array(src, convert, value); }

  static handle cast(const f3d::vector3_t& src, return_value_policy, handle /* parent */)
  {
    return Py_BuildValue("ddd", src[0], src[1], src[2]);
  }

  PYBIND11_TYPE_CASTER(f3d::vector3_t, const_name("f3d.vector3_t"));
};

PYBIND11_MODULE(pyf3d, module)
{
  module.doc() = "f3d library bindings";

  // f3d::image
  py::class_<f3d::image> image(module, "image");

  py::enum_<f3d::image::SaveFormat>(image, "SaveFormat")
    .value("PNG", f3d::image::SaveFormat::PNG)
    .value("JPG", f3d::image::SaveFormat::JPG)
    .value("TIF", f3d::image::SaveFormat::TIF)
    .value("BMP", f3d::image::SaveFormat::BMP)
    .export_values();

  py::enum_<f3d::image::ChannelType>(image, "ChannelType")
    .value("BYTE", f3d::image::ChannelType::BYTE)
    .value("SHORT", f3d::image::ChannelType::SHORT)
    .value("FLOAT", f3d::image::ChannelType::FLOAT)
    .export_values();

  auto setImageBytes = [](f3d::image& img, const py::bytes& data)
  {
    const py::buffer_info info(py::buffer(data).request());
    size_t expectedSize =
      img.getChannelCount() * img.getWidth() * img.getHeight() * img.getChannelTypeSize();
    if (info.itemsize != 1 || info.size != expectedSize)
    {
      throw py::value_error();
    }
    img.setContent(info.ptr);
  };

  auto getImageBytes = [](const f3d::image& img)
  {
    size_t expectedSize =
      img.getChannelCount() * img.getWidth() * img.getHeight() * img.getChannelTypeSize();
    return py::bytes(static_cast<char*>(img.getContent()), expectedSize);
  };

  image //
    .def(py::init<>())
    .def(py::init<const std::string&>())
    .def(py::init<unsigned int, unsigned int, unsigned int, f3d::image::ChannelType>())
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def_property_readonly("width", &f3d::image::getWidth)
    .def_property_readonly("height", &f3d::image::getHeight)
    .def_property_readonly("channel_count", &f3d::image::getChannelCount)
    .def_property_readonly("channel_type", &f3d::image::getChannelType)
    .def_property_readonly("channel_type_size", &f3d::image::getChannelTypeSize)
    .def_property("content", getImageBytes, setImageBytes)
    .def("compare", &f3d::image::compare)
    .def(
      "save", &f3d::image::save, py::arg("path"), py::arg("format") = f3d::image::SaveFormat::PNG);

  // f3d::options
  py::class_<f3d::options> options(module, "options");

  options //
    .def(py::init<>())
    .def("__setitem__", py::overload_cast<const std::string&, bool>(&f3d::options::set))
    .def("__setitem__", py::overload_cast<const std::string&, int>(&f3d::options::set))
    .def("__setitem__", py::overload_cast<const std::string&, double>(&f3d::options::set))
    .def(
      "__setitem__", py::overload_cast<const std::string&, const std::string&>(&f3d::options::set))
    .def("__setitem__",
      py::overload_cast<const std::string&, const std::vector<int>&>(&f3d::options::set))
    .def("__setitem__",
      py::overload_cast<const std::string&, const std::vector<double>&>(&f3d::options::set))
    .def("__getitem__",
      [](f3d::options& opts, const std::string key)
      {
#define TRY(getter)                                                                                \
  try                                                                                              \
  {                                                                                                \
    auto v = getter(key);                                                                          \
    return py::cast(v);                                                                            \
  }                                                                                                \
  catch (const f3d::options::inexistent_exception&)                                                \
  {                                                                                                \
    throw pybind11::key_error(key);                                                                \
  }                                                                                                \
  catch (const f3d::options::incompatible_exception&)                                              \
  {                                                                                                \
  }
        TRY(opts.getAsBoolRef)
        TRY(opts.getAsIntRef)
        TRY(opts.getAsDoubleRef)
        TRY(opts.getAsStringRef)
        TRY(opts.getAsDoubleVectorRef)
        TRY(opts.getAsIntVectorRef)
        throw pybind11::key_error(key);
#undef TRY
      })
    .def("__len__", [](f3d::options& opts) { return opts.getNames().size(); })
    .def(
      "__iter__",
      [](f3d::options& opts)
      {
        const auto names = py::cast(opts.getNames()); // won't work without cast
        return make_iterator(names);
      },
      py::keep_alive<0, 1>())
    .def("keys", &f3d::options::getNames) // to do `dict(options)`
    .def("toggle", &f3d::options::toggle)
    .def("is_same", &f3d::options::isSame)
    .def("get_closest_option", &f3d::options::getClosestOption)
    .def("copy", &f3d::options::copy);

  // f3d::utils
  py::class_<f3d::utils> utils(module, "utils");

  utils //
    .def_static("text_distance", &f3d::utils::textDistance);

  // f3d::interactor
  py::class_<f3d::interactor, std::unique_ptr<f3d::interactor, py::nodelete> > interactor(
    module, "interactor");
  interactor //
    .def("set_key_press_callback", &f3d::interactor::setKeyPressCallBack,
      "Define a callback triggered when a key is pressed")
    .def("set_drop_files_callback", &f3d::interactor::setDropFilesCallBack,
      "Define a callback triggered when files are dropped")
    .def("create_timer_callback", &f3d::interactor::createTimerCallBack,
      "Define a callback triggered at a fixed timestep")
    .def("remove_timer_callback", &f3d::interactor::removeTimerCallBack,
      "Delete a previously created timer callback")
    .def("toggle_animation", &f3d::interactor::toggleAnimation, "Toggle the animation")
    .def("start_animation", &f3d::interactor::startAnimation, "Start the animation")
    .def("stop_animation", &f3d::interactor::stopAnimation, "Stop the animation")
    .def("is_playing_animation", &f3d::interactor::isPlayingAnimation,
      "Returns True if the animation is currently started")
    .def("enable_camera_movement", &f3d::interactor::enableCameraMovement,
      "Enable the camera interaction")
    .def("disable_camera_movement", &f3d::interactor::disableCameraMovement,
      "Disable the camera interaction")
    .def("play_interaction", &f3d::interactor::playInteraction, "Play an interaction file")
    .def("record_interaction", &f3d::interactor::recordInteraction, "Record an interaction file")
    .def("start", &f3d::interactor::start, "Start the interactor")
    .def("stop", &f3d::interactor::start, "Stop the interactor")
    .def_static("get_default_interactions_info", &f3d::interactor::getDefaultInteractionsInfo);

  // f3d::loader
  py::class_<f3d::loader, std::unique_ptr<f3d::loader, py::nodelete> > loader(module, "loader");
  loader //
    .def("has_geometry_reader", &f3d::loader::hasGeometryReader)
    .def("load_geometry", &f3d::loader::loadGeometry, "load geometry to a default scene",
      py::arg("file_path"), py::arg("reset") = false)
    .def("has_scene_reader", &f3d::loader::hasSceneReader)
    .def("load_scene", &f3d::loader::loadScene, "Load a specific full scene file");

  // f3d::camera
  py::class_<f3d::camera, std::unique_ptr<f3d::camera, py::nodelete> > camera(module, "camera");
  camera //
    .def_property(
      "position", [](f3d::camera& cam) { return cam.getPosition(); }, &f3d::camera::setPosition)
    .def_property(
      "focal_point", [](f3d::camera& cam) { return cam.getFocalPoint(); },
      &f3d::camera::setFocalPoint)
    .def_property(
      "view_up", [](f3d::camera& cam) { return cam.getViewUp(); }, &f3d::camera::setViewUp)
    .def_property(
      "view_angle", [](f3d::camera& cam) { return cam.getViewAngle(); }, &f3d::camera::setViewAngle)
    .def_property(
      "state", [](f3d::camera& cam) { return cam.getState(); }, &f3d::camera::setState)
    .def("dolly", &f3d::camera::dolly)
    .def("roll", &f3d::camera::roll)
    .def("azimuth", &f3d::camera::azimuth)
    .def("yaw", &f3d::camera::yaw)
    .def("elevation", &f3d::camera::elevation)
    .def("pitch", &f3d::camera::pitch)
    .def("set_current_as_default", &f3d::camera::setCurrentAsDefault)
    .def("reset_to_default", &f3d::camera::resetToDefault)
    .def("reset_to_bounds", &f3d::camera::resetToBounds, py::arg("zoom_factor") = 0.9);

  py::class_<f3d::camera_state_t>(module, "camera_state_t")
    .def(py::init<>())
    .def(py::init<const f3d::point3_t&, const f3d::point3_t&, const f3d::vector3_t&,
      const f3d::angle_deg_t&>())
    .def_readwrite("pos", &f3d::camera_state_t::pos)
    .def_readwrite("foc", &f3d::camera_state_t::foc)
    .def_readwrite("up", &f3d::camera_state_t::up)
    .def_readwrite("angle", &f3d::camera_state_t::angle);

  // f3d::window
  py::class_<f3d::window, std::unique_ptr<f3d::window, py::nodelete> > window(module, "window");

  py::enum_<f3d::window::Type>(window, "Type")
    .value("NONE", f3d::window::Type::NONE)
    .value("NATIVE", f3d::window::Type::NATIVE)
    .value("NATIVE_OFFSCREEN", f3d::window::Type::NATIVE_OFFSCREEN)
    .value("EXTERNAL", f3d::window::Type::EXTERNAL)
    .export_values();

  window //
    .def_property_readonly("type", &f3d::window::getType)
    .def_property_readonly("camera", &f3d::window::getCamera, py::return_value_policy::reference)
    .def_property(
      "size",
      [](const f3d::window& win) { return std::make_pair(win.getWidth(), win.getHeight()); },
      [](f3d::window& win, std::pair<int, int> wh) { win.setSize(wh.first, wh.second); })
    .def_property("width", &f3d::window::getWidth,
      [](f3d::window& win, int w) { win.setSize(w, win.getHeight()); })
    .def_property("height", &f3d::window::getHeight,
      [](f3d::window& win, int h) { win.setSize(win.getWidth(), h); })
    .def("render", &f3d::window::render, "Render the window")
    .def("render_to_image", &f3d::window::renderToImage, "Render the window to an image",
      py::arg("no_background") = false)
    .def("set_position", &f3d::window::setPosition)
    .def("set_icon", &f3d::window::setIcon,
      "Set the icon of the window using a memory buffer representing a PNG file")
    .def("set_window_name", &f3d::window::setWindowName, "Set the window name")
    .def("get_world_from_display", &f3d::window::getWorldFromDisplay,
      "Get world coordinate point from display coordinate")
    .def("get_display_from_world", &f3d::window::getDisplayFromWorld,
      "Get display coordinate point from world coordinate");

  // f3d::engine
  py::class_<f3d::engine> engine(module, "engine");

  engine //
    .def(py::init<f3d::window::Type>(), py::arg("window_type") = f3d::window::Type::NATIVE)
    .def("set_cache_path", &f3d::engine::setCachePath, "Set the cache path directory")
    .def_property("options", &f3d::engine::getOptions,
      py::overload_cast<const f3d::options&>(&f3d::engine::setOptions),
      py::return_value_policy::reference)
    .def_property_readonly("window", &f3d::engine::getWindow, py::return_value_policy::reference)
    .def_property_readonly("loader", &f3d::engine::getLoader, py::return_value_policy::reference)
    .def_property_readonly(
      "interactor", &f3d::engine::getInteractor, py::return_value_policy::reference)
    .def_static("load_plugin", &f3d::engine::loadPlugin, "Load a plugin")
    .def_static(
      "autoload_plugins", &f3d::engine::autoloadPlugins, "Automatically load internal plugins")
    .def_static("get_lib_info", &f3d::engine::getLibInfo)
    .def_static("get_readers_info", &f3d::engine::getReadersInfo)
    .def_static("get_plugins_list", &f3d::engine::getPluginsList);

// deprecated functions, will be removed in the next major release, F3D v3.0.0
#ifndef F3D_NO_DEPRECATED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  image //
    .def("setResolution", &f3d::image::setResolution,
      "DEPRECATED: use the appropriate constructor instead.")
    .def("setChannelCount", &f3d::image::setResolution,
      "DEPRECATED: use the appropriate constructor instead.")
    .def(
      "setData", [=](f3d::image& img, const py::bytes& data) { setImageBytes(img, data); },
      "DEPRECATED: use setContent instead")
    .def(
      "getData", [=](const f3d::image& img) { return getImageBytes(img); },
      "DEPRECATED: use getContent instead.")
    .def("getWidth", &f3d::image::getWidth, "DEPRECATED")
    .def("getHeight", &f3d::image::getHeight, "DEPRECATED")
    .def("getChannelCount", &f3d::image::getChannelCount, "DEPRECATED")
    .def("getChannelType", &f3d::image::getChannelType, "DEPRECATED")
    .def("getChannelTypeSize", &f3d::image::getChannelTypeSize, "DEPRECATED")
    .def("setContent", setImageBytes, "DEPRECATED")
    .def("getContent", getImageBytes, "DEPRECATED");

  options //
    .def("set", py::overload_cast<const std::string&, bool>(&f3d::options::set),
      "Set a boolean option", "DEPRECATED")
    .def("set", py::overload_cast<const std::string&, int>(&f3d::options::set),
      "Set an integer option", "DEPRECATED")
    .def("set", py::overload_cast<const std::string&, double>(&f3d::options::set),
      "Set a floating point option", "DEPRECATED")
    .def("set", py::overload_cast<const std::string&, const std::string&>(&f3d::options::set),
      "Set a string option", "DEPRECATED")
    .def("set", py::overload_cast<const std::string&, const std::vector<int>&>(&f3d::options::set),
      "Set an array of integers option", "DEPRECATED")
    .def("set",
      py::overload_cast<const std::string&, const std::vector<double>&>(&f3d::options::set),
      "Set an array of floating points option", "DEPRECATED")
    .def("getAsBool", &f3d::options::getAsBool, "DEPRECATED")
    .def("getAsInt", &f3d::options::getAsInt, "DEPRECATED")
    .def("getAsDouble", &f3d::options::getAsDouble, "DEPRECATED")
    .def("getAsString", &f3d::options::getAsString, "DEPRECATED")
    .def("getAsIntVector", &f3d::options::getAsIntVector, "DEPRECATED")
    .def("getAsDoubleVector", &f3d::options::getAsDoubleVector, "DEPRECATED")
    .def("isSame", &f3d::options::isSame, "DEPRECATED")
    .def("getNames", &f3d::options::getNames, "DEPRECATED")
    .def("getClosestOption", &f3d::options::getClosestOption, "DEPRECATED");

  utils //
    .def_static("textDistance", &f3d::utils::textDistance, "DEPRECATED");

  interactor //
    .def("setKeyPressCallBack", &f3d::interactor::setKeyPressCallBack,
      "Define a callback triggered when a key is pressed", "DEPRECATED")
    .def("setDropFilesCallBack", &f3d::interactor::setDropFilesCallBack,
      "Define a callback triggered when files are dropped", "DEPRECATED")
    .def("createTimerCallBack", &f3d::interactor::createTimerCallBack,
      "Define a callback triggered at a fixed timestep", "DEPRECATED")
    .def("removeTimerCallBack", &f3d::interactor::removeTimerCallBack,
      "Delete a previously created timer callback", "DEPRECATED")
    .def("toggleAnimation", &f3d::interactor::toggleAnimation, "Toggle the animation", "DEPRECATED")
    .def("startAnimation", &f3d::interactor::startAnimation, "Start the animation", "DEPRECATED")
    .def("stopAnimation", &f3d::interactor::stopAnimation, "Stop the animation", "DEPRECATED")
    .def("isPlayingAnimation", &f3d::interactor::isPlayingAnimation,
      "Returns True if the animation is currently started", "DEPRECATED")
    .def("enableCameraMovement", &f3d::interactor::enableCameraMovement,
      "Enable the camera interaction", "DEPRECATED")
    .def("disableCameraMovement", &f3d::interactor::disableCameraMovement,
      "Disable the camera interaction", "DEPRECATED")
    .def("playInteraction", &f3d::interactor::playInteraction, "Play an interaction file",
      "DEPRECATED")
    .def("recordInteraction", &f3d::interactor::recordInteraction, "Record an interaction file",
      "DEPRECATED")
    .def_static(
      "getDefaultInteractionsInfo", &f3d::interactor::getDefaultInteractionsInfo, "DEPRECATED");

  loader //
    .def("hasGeometryReader", &f3d::loader::hasGeometryReader, "DEPRECATED")
    .def(
      "loadGeometry", &f3d::loader::loadGeometry, "load geometry to a default scene", "DEPRECATED")
    .def("hasSceneReader", &f3d::loader::hasSceneReader, "DEPRECATED")
    .def("loadScene", &f3d::loader::loadScene, "Load a specific full scene file", "DEPRECATED");

  camera //
    .def("setPosition", &f3d::camera::setPosition, "DEPRECATED: use position property setter")
    .def("getPosition", py::overload_cast<>(&f3d::camera::getPosition),
      "DEPRECATED: use position property getter")
    .def("setFocalPoint", &f3d::camera::setFocalPoint, "DEPRECATED")
    .def("getFocalPoint", py::overload_cast<>(&f3d::camera::getFocalPoint), "DEPRECATED")
    .def("setViewUp", &f3d::camera::setViewUp, "DEPRECATED")
    .def("getViewUp", py::overload_cast<>(&f3d::camera::getViewUp), "DEPRECATED")
    .def("setViewAngle", &f3d::camera::setViewAngle, "DEPRECATED")
    .def("getViewAngle", py::overload_cast<>(&f3d::camera::getViewAngle), "DEPRECATED")
    .def("getViewAngle", py::overload_cast<f3d::angle_deg_t&>(&f3d::camera::getViewAngle),
      "DEPRECATED")
    .def("setState", &f3d::camera::setState, "DEPRECATED")
    .def("getState", py::overload_cast<>(&f3d::camera::getState), "DEPRECATED")
    .def("getState", py::overload_cast<f3d::camera_state_t&>(&f3d::camera::getState), "DEPRECATED")
    .def("setCurrentAsDefault", &f3d::camera::setCurrentAsDefault, "DEPRECATED")
    .def("resetToDefault", &f3d::camera::resetToDefault, "DEPRECATED")
    .def("resetToBounds", &f3d::camera::resetToBounds, py::arg("zoomFactor") = 0.9, "DEPRECATED");

  window //
    .def("getType", &f3d::window::getType, "DEPRECATED")
    .def("getCamera", &f3d::window::getCamera, py::return_value_policy::reference, "DEPRECATED")
    .def("render", &f3d::window::render, "Render the window", "DEPRECATED")
    .def("renderToImage", &f3d::window::renderToImage, "Render the window to an image",
      py::arg("noBackground") = false, "DEPRECATED")
    .def("setSize", &f3d::window::setSize, "Set the window size", "DEPRECATED")
    .def("getWidth", &f3d::window::getWidth, "Get the window width", "DEPRECATED")
    .def("getHeight", &f3d::window::getHeight, "Get the window height", "DEPRECATED")
    .def("setPosition", &f3d::window::setPosition, "DEPRECATED")
    .def("setIcon", &f3d::window::setIcon,
      "Set the icon of the window using a memory buffer representing a PNG file", "DEPRECATED")
    .def("setWindowName", &f3d::window::setWindowName, "Set the window name", "DEPRECATED")
    .def("getWorldFromDisplay", &f3d::window::getWorldFromDisplay,
      "Get world coordinate point from display coordinate", "DEPRECATED")
    .def("getDisplayFromWorld", &f3d::window::getDisplayFromWorld,
      "Get display coordinate point from world coordinate", "DEPRECATED");

  engine //
    .def("setCachePath", &f3d::engine::setCachePath, "Set the cache path directory", "DEPRECATED")
    .def(
      "setOptions", py::overload_cast<const f3d::options&>(&f3d::engine::setOptions), "DEPRECATED")
    .def("getOptions", &f3d::engine::getOptions, py::return_value_policy::reference, "DEPRECATED")
    .def("getWindow", &f3d::engine::getWindow, py::return_value_policy::reference, "DEPRECATED")
    .def("getLoader", &f3d::engine::getLoader, py::return_value_policy::reference, "DEPRECATED")
    .def("getInteractor", &f3d::engine::getInteractor, py::return_value_policy::reference,
      "DEPRECATED")
    .def_static("loadPlugin", &f3d::engine::loadPlugin, "Load a plugin", "DEPRECATED")
    .def_static("autoloadPlugins", &f3d::engine::autoloadPlugins,
      "Automatically load internal plugins", "DEPRECATED")
    .def_static("getLibInfo", &f3d::engine::getLibInfo, "DEPRECATED")
    .def_static("getReadersInfo", &f3d::engine::getReadersInfo, "DEPRECATED")
    .def_static("getPluginsList", &f3d::engine::getPluginsList, "DEPRECATED");
#pragma GCC diagnostic pop
#endif
}
