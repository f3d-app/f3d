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

PYBIND11_MODULE(f3d, module)
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

  image.def(py::init<>())
    .def(py::init<const std::string&>())
    .def(py::init<unsigned int, unsigned int, unsigned int, f3d::image::ChannelType>())
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def("getWidth", &f3d::image::getWidth)
    .def("getHeight", &f3d::image::getHeight)
    .def("getChannelCount", &f3d::image::getChannelCount)
    .def("getChannelType", &f3d::image::getChannelType)
    .def("getChannelTypeSize", &f3d::image::getChannelTypeSize)
    .def("setContent", setImageBytes)
    .def("getContent", getImageBytes)
    .def("compare", &f3d::image::compare)
    .def(
      "save", &f3d::image::save, py::arg("path"), py::arg("format") = f3d::image::SaveFormat::PNG);

  // f3d::options
  py::class_<f3d::options>(module, "options")
    .def(py::init<>())
    .def("set", py::overload_cast<const std::string&, bool>(&f3d::options::set),
      "Set a boolean option")
    .def("set", py::overload_cast<const std::string&, int>(&f3d::options::set),
      "Set an integer option")
    .def("set", py::overload_cast<const std::string&, double>(&f3d::options::set),
      "Set a floating point option")
    .def("set", py::overload_cast<const std::string&, const std::string&>(&f3d::options::set),
      "Set a string option")
    .def("set", py::overload_cast<const std::string&, const std::vector<int>&>(&f3d::options::set),
      "Set an array of integers option")
    .def("set",
      py::overload_cast<const std::string&, const std::vector<double>&>(&f3d::options::set),
      "Set an array of floating points option")
    .def("getAsBool", &f3d::options::getAsBool)
    .def("getAsInt", &f3d::options::getAsInt)
    .def("getAsDouble", &f3d::options::getAsDouble)
    .def("getAsString", &f3d::options::getAsString)
    .def("getAsIntVector", &f3d::options::getAsIntVector)
    .def("getAsDoubleVector", &f3d::options::getAsDoubleVector)
    .def("toggle", &f3d::options::toggle)
    .def("isSame", &f3d::options::isSame)
    .def("copy", &f3d::options::copy)
    .def("getNames", &f3d::options::getNames)
    .def("getClosestOption", &f3d::options::getClosestOption);

  // f3d::utils
  py::class_<f3d::utils>(module, "utils").def_static("textDistance", &f3d::utils::textDistance);

  // f3d::interactor
  py::class_<f3d::interactor, std::unique_ptr<f3d::interactor, py::nodelete> >(module, "interactor")
    .def("setKeyPressCallBack", &f3d::interactor::setKeyPressCallBack,
      "Define a callback triggered when a key is pressed")
    .def("setDropFilesCallBack", &f3d::interactor::setDropFilesCallBack,
      "Define a callback triggered when files are dropped")
    .def("createTimerCallBack", &f3d::interactor::createTimerCallBack,
      "Define a callback triggered at a fixed timestep")
    .def("removeTimerCallBack", &f3d::interactor::removeTimerCallBack,
      "Delete a previously created timer callback")
    .def("toggleAnimation", &f3d::interactor::toggleAnimation, "Toggle the animation")
    .def("startAnimation", &f3d::interactor::startAnimation, "Start the animation")
    .def("stopAnimation", &f3d::interactor::stopAnimation, "Stop the animation")
    .def("isPlayingAnimation", &f3d::interactor::isPlayingAnimation,
      "Returns True if the animation is currently started")
    .def("enableCameraMovement", &f3d::interactor::enableCameraMovement,
      "Enable the camera interaction")
    .def("disableCameraMovement", &f3d::interactor::disableCameraMovement,
      "Disable the camera interaction")
    .def("playInteraction", &f3d::interactor::playInteraction, "Play an interaction file")
    .def("recordInteraction", &f3d::interactor::recordInteraction, "Record an interaction file")
    .def("start", &f3d::interactor::start, "Start the interactor")
    .def("stop", &f3d::interactor::start, "Stop the interactor")
    .def_static("getDefaultInteractionsInfo", &f3d::interactor::getDefaultInteractionsInfo);

  // f3d::loader
  py::class_<f3d::loader, std::unique_ptr<f3d::loader, py::nodelete> > loader(module, "loader");
  loader.def("hasGeometryReader", &f3d::loader::hasGeometryReader)
    .def("loadGeometry", &f3d::loader::loadGeometry, "load geometry to a default scene")
    .def("hasSceneReader", &f3d::loader::hasSceneReader)
    .def("loadScene", &f3d::loader::loadScene, "Load a specific full scene file");

  // f3d::camera
  py::class_<f3d::camera, std::unique_ptr<f3d::camera, py::nodelete> >(module, "camera")
    .def("setPosition", &f3d::camera::setPosition)
    .def("getPosition", py::overload_cast<>(&f3d::camera::getPosition))
    .def("setFocalPoint", &f3d::camera::setFocalPoint)
    .def("getFocalPoint", py::overload_cast<>(&f3d::camera::getFocalPoint))
    .def("setViewUp", &f3d::camera::setViewUp)
    .def("getViewUp", py::overload_cast<>(&f3d::camera::getViewUp))
    .def("setViewAngle", &f3d::camera::setViewAngle)
    .def("getViewAngle", py::overload_cast<>(&f3d::camera::getViewAngle))
    .def("getViewAngle", py::overload_cast<f3d::angle_deg_t&>(&f3d::camera::getViewAngle))
    .def("dolly", &f3d::camera::dolly)
    .def("roll", &f3d::camera::roll)
    .def("azimuth", &f3d::camera::azimuth)
    .def("yaw", &f3d::camera::yaw)
    .def("elevation", &f3d::camera::elevation)
    .def("pitch", &f3d::camera::pitch)
    .def("setState", &f3d::camera::setState)
    .def("getState", py::overload_cast<>(&f3d::camera::getState))
    .def("getState", py::overload_cast<f3d::camera_state_t&>(&f3d::camera::getState))
    .def("setCurrentAsDefault", &f3d::camera::setCurrentAsDefault)
    .def("resetToDefault", &f3d::camera::resetToDefault)
    .def("resetToBounds", &f3d::camera::resetToBounds, py::arg("zoomFactor") = 0.9);

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

  window.def("getType", &f3d::window::getType)
    .def("getCamera", &f3d::window::getCamera, py::return_value_policy::reference)
    .def("render", &f3d::window::render, "Render the window")
    .def("renderToImage", &f3d::window::renderToImage, "Render the window to an image",
      py::arg("noBackground") = false)
    .def("setSize", &f3d::window::setSize, "Set the window size")
    .def("getWidth", &f3d::window::getWidth, "Get the window width")
    .def("getHeight", &f3d::window::getHeight, "Get the window height")
    .def("setPosition", &f3d::window::setPosition)
    .def("setIcon", &f3d::window::setIcon,
      "Set the icon of the window using a memory buffer representing a PNG file")
    .def("setWindowName", &f3d::window::setWindowName, "Set the window name")
    .def("getWorldFromDisplay", &f3d::window::getWorldFromDisplay,
      "Get world coordinate point from display coordinate")
    .def("getDisplayFromWorld", &f3d::window::getDisplayFromWorld,
      "Get display coordinate point from world coordinate");

  // f3d::engine
  py::class_<f3d::engine> engine(module, "engine");

  engine.def(py::init<f3d::window::Type>(), py::arg("windowType") = f3d::window::Type::NATIVE)
    .def("setCachePath", &f3d::engine::setCachePath, "Set the cache path directory")
    .def("setOptions", py::overload_cast<const f3d::options&>(&f3d::engine::setOptions))
    .def("getOptions", &f3d::engine::getOptions, py::return_value_policy::reference)
    .def("getWindow", &f3d::engine::getWindow, py::return_value_policy::reference)
    .def("getLoader", &f3d::engine::getLoader, py::return_value_policy::reference)
    .def("getInteractor", &f3d::engine::getInteractor, py::return_value_policy::reference)
    .def_static("loadPlugin", &f3d::engine::loadPlugin, "Load a plugin")
    .def_static(
      "autoloadPlugins", &f3d::engine::autoloadPlugins, "Automatically load internal plugins")
    .def_static("getLibInfo", &f3d::engine::getLibInfo)
    .def_static("getReadersInfo", &f3d::engine::getReadersInfo)
    .def_static("getPluginsList", &f3d::engine::getPluginsList);

// deprecated functions, will be removed in the next major release
#ifndef F3D_NO_DEPRECATED
  image.def("setResolution",
    [](f3d::image& img, unsigned int width, unsigned height)
    {
      PyErr_WarnEx(PyExc_DeprecationWarning,
        "setResolution is deprecated, use the appropriate constructor instead.", 1);
      return img.setResolution(width, height);
    });

  image.def("setChannelCount",
    [](f3d::image& img, unsigned int channels)
    {
      PyErr_WarnEx(PyExc_DeprecationWarning,
        "setChannelCount is deprecated, use the appropriate constructor instead.", 1);
      return img.setChannelCount(channels);
    });

  image.def("setData", setImageBytes);
  image.def("getData", getImageBytes);
#endif
}
