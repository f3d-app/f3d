#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "camera.h"
#include "engine.h"
#include "image.h"
#include "interactor.h"
#include "loader.h"
#include "log.h"
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
  bool load(handle src, bool convert)
  {
    return load_array(src, convert, value);
  }

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
  bool load(handle src, bool convert)
  {
    return load_array(src, convert, value);
  }

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
  py::class_<f3d::image> image(module, "Image");

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

  auto getFileBytes = [](const f3d::image& img, f3d::image::SaveFormat format)
  {
    std::vector<unsigned char> result = img.saveBuffer(format);
    return py::bytes(reinterpret_cast<char*>(result.data()), result.size());
  };

  image //
    .def(py::init<>())
    .def(py::init<const std::string&>())
    .def(py::init<unsigned int, unsigned int, unsigned int, f3d::image::ChannelType>())
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def_static("supported_formats", &f3d::image::getSupportedFormats)
    .def_property_readonly("width", &f3d::image::getWidth)
    .def_property_readonly("height", &f3d::image::getHeight)
    .def_property_readonly("channel_count", &f3d::image::getChannelCount)
    .def_property_readonly("channel_type", &f3d::image::getChannelType)
    .def_property_readonly("channel_type_size", &f3d::image::getChannelTypeSize)
    .def_property("content", getImageBytes, setImageBytes)
    .def("compare", &f3d::image::compare)
    .def(
      "save", &f3d::image::save, py::arg("path"), py::arg("format") = f3d::image::SaveFormat::PNG)
    .def("save_buffer", getFileBytes, py::arg("format") = f3d::image::SaveFormat::PNG)
    .def("_repr_png_",
      [&](const f3d::image& img) { return getFileBytes(img, f3d::image::SaveFormat::PNG); })
    .def("to_terminal_text", [](const f3d::image& img) { return img.toTerminalText(); })
    .def("set_metadata", &f3d::image::setMetadata)
    .def("get_metadata",
      [](const f3d::image& img, const std::string& key)
      {
        try
        {
          return img.getMetadata(key);
        }
        catch (const f3d::image::metadata_exception&)
        {
          throw py::key_error(key);
        }
      })
    .def("all_metadata", &f3d::image::allMetadata)
    .def("normalized_pixel", &f3d::image::getNormalizedPixel);

  // f3d::options
  py::class_<f3d::options> options(module, "Options");

  options //
    .def(py::init<>())
    .def("__setitem__",
      [](f3d::options& opts, const std::string& key, const f3d::option_variant_t& value)
      {
        try
        {
          opts.set(key, value);
        }
        catch (const f3d::options::inexistent_exception&)
        {
          throw py::key_error(key);
        }
        catch (const f3d::options::incompatible_exception&)
        {
          throw py::attribute_error(key);
        }
      })
    .def("__getitem__",
      [](f3d::options& opts, const std::string& key)
      {
        try
        {
          return opts.get(key);
        }
        catch (const f3d::options::inexistent_exception&)
        {
          throw py::key_error(key);
        }
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
  py::class_<f3d::utils> utils(module, "Utils");

  utils //
    .def_static("text_distance", &f3d::utils::textDistance);

  // f3d::interactor
  py::class_<f3d::interactor, std::unique_ptr<f3d::interactor, py::nodelete>> interactor(
    module, "Interactor");
  interactor //
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

  // f3d::mesh_t
  py::class_<f3d::mesh_t>(module, "Mesh")
    .def(py::init<>())
    .def(py::init<const std::vector<float>&, const std::vector<float>&, const std::vector<float>&,
           const std::vector<unsigned int>&, const std::vector<unsigned int>&>(),
      py::arg("points"), py::arg("normals") = std::vector<float>(),
      py::arg("texture_coordinates") = std::vector<float>(),
      py::arg("face_sides") = std::vector<unsigned int>(),
      py::arg("face_indices") = std::vector<unsigned int>())
    .def_readwrite("points", &f3d::mesh_t::points)
    .def_readwrite("normals", &f3d::mesh_t::normals)
    .def_readwrite("texture_coordinates", &f3d::mesh_t::texture_coordinates)
    .def_readwrite("face_sides", &f3d::mesh_t::face_sides)
    .def_readwrite("face_indices", &f3d::mesh_t::face_indices);

  // f3d::loader
  py::class_<f3d::loader, std::unique_ptr<f3d::loader, py::nodelete>> loader(module, "Loader");
  loader //
    .def("has_geometry_reader", &f3d::loader::hasGeometryReader)
    .def("load_geometry", py::overload_cast<const std::string&, bool>(&f3d::loader::loadGeometry),
      "load geometry to a default scene", py::arg("file_path"), py::arg("reset") = false)
    .def("has_scene_reader", &f3d::loader::hasSceneReader)
    .def("load_scene", &f3d::loader::loadScene, "Load a specific full scene file")
    .def("load_geometry", py::overload_cast<const f3d::mesh_t&, bool>(&f3d::loader::loadGeometry),
      "Load a surfacic mesh from memory", py::arg("mesh"), py::arg("reset") = false);

  // f3d::camera
  py::class_<f3d::camera, std::unique_ptr<f3d::camera, py::nodelete>> camera(module, "Camera");
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
    .def("pan", &f3d::camera::pan, py::arg("right"), py::arg("up"), py::arg("forward") = 0.0)
    .def("zoom", &f3d::camera::zoom)
    .def("roll", &f3d::camera::roll)
    .def("azimuth", &f3d::camera::azimuth)
    .def("yaw", &f3d::camera::yaw)
    .def("elevation", &f3d::camera::elevation)
    .def("pitch", &f3d::camera::pitch)
    .def("set_current_as_default", &f3d::camera::setCurrentAsDefault)
    .def("reset_to_default", &f3d::camera::resetToDefault)
    .def("reset_to_bounds", &f3d::camera::resetToBounds, py::arg("zoom_factor") = 0.9);

  py::class_<f3d::camera_state_t>(module, "CameraState")
    .def(py::init<>())
    .def(py::init<const f3d::point3_t&, const f3d::point3_t&, const f3d::vector3_t&,
      const f3d::angle_deg_t&>())
    .def_readwrite("pos", &f3d::camera_state_t::pos)
    .def_readwrite("foc", &f3d::camera_state_t::foc)
    .def_readwrite("up", &f3d::camera_state_t::up)
    .def_readwrite("angle", &f3d::camera_state_t::angle);

  // f3d::window
  py::class_<f3d::window, std::unique_ptr<f3d::window, py::nodelete>> window(module, "Window");

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
  py::class_<f3d::engine> engine(module, "Engine");

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
    .def_static("get_plugins_list", &f3d::engine::getPluginsList);

  // f3d::log
  py::class_<f3d::log> log(module, "Log");

  log //
    .def_static("set_verbose_level", &f3d::log::setVerboseLevel, py::arg("level"),
      py::arg("force_std_err") = false)
    .def_static("set_use_coloring", &f3d::log::setUseColoring)
    .def_static("print",
      [](f3d::log::VerboseLevel& level, const std::string& message)
      { f3d::log::print(level, message); });

  py::enum_<f3d::log::VerboseLevel>(log, "VerboseLevel")
    .value("DEBUG", f3d::log::VerboseLevel::DEBUG)
    .value("INFO", f3d::log::VerboseLevel::INFO)
    .value("WARN", f3d::log::VerboseLevel::WARN)
    .value("ERROR", f3d::log::VerboseLevel::ERROR)
    .value("QUIET", f3d::log::VerboseLevel::QUIET)
    .export_values();
}
