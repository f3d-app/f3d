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
#include "window.h"

namespace py = pybind11;

PYBIND11_MODULE(f3d, module)
{
  module.doc() = "f3d library bindings";

  // types
  py::class_<f3d::point3_t>(module, "point3_t").def(py::init<double, double, double>());
  py::class_<f3d::vector3_t>(module, "vector3_t").def(py::init<double, double, double>());

  // f3d::image
  py::class_<f3d::image>(module, "image")
    .def(py::init<>())
    .def(py::init<const std::string&>())
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def("getWidth", &f3d::image::getWidth)
    .def("getHeight", &f3d::image::getHeight)
    .def("setResolution", &f3d::image::setResolution)
    .def("getChannelCount", &f3d::image::getChannelCount)
    .def("setChannelCount", &f3d::image::setChannelCount)
    .def("setData",
      [](f3d::image& img, const py::bytes& data)
      {
        const py::buffer_info info(py::buffer(data).request());
        if (info.itemsize != 1 ||
          info.size != img.getChannelCount() * img.getWidth() * img.getHeight())
        {
          throw py::value_error();
        }
        img.setData((unsigned char*)info.ptr);
      })
    .def("getData",
      [](const f3d::image& img)
      {
        return py::bytes(
          (char*)img.getData(), img.getChannelCount() * img.getWidth() * img.getHeight());
      })
    .def("compare", &f3d::image::compare)
    .def("save", &f3d::image::save);

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
    .def("set", py::overload_cast<const std::string&, const std::vector<int>&>(&f3d::options::set),
      "Set an array of integers option")
    .def("getAsBool", &f3d::options::getAsBool)
    .def("getAsInt", &f3d::options::getAsInt)
    .def("getAsDouble", &f3d::options::getAsDouble)
    .def("getAsString", &f3d::options::getAsString)
    .def("getAsIntVector", &f3d::options::getAsIntVector)
    .def("getAsDoubleVector", &f3d::options::getAsDoubleVector)
    .def("getNames", &f3d::options::getNames)
    .def("isSame", &f3d::options::isSame)
    .def("copy", &f3d::options::copy);

  // f3d::interactor
  py::class_<f3d::interactor, std::unique_ptr<f3d::interactor, py::nodelete> >(module, "interactor")
    .def("start", &f3d::interactor::start, "Start the interactor")
    .def("stop", &f3d::interactor::start, "Stop the interactor")
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
    .def("recordInteraction", &f3d::interactor::recordInteraction, "Record an interaction file");

  // f3d::loader
  py::class_<f3d::loader, std::unique_ptr<f3d::loader, py::nodelete> > loader(module, "loader");

  py::enum_<f3d::loader::LoadFileEnum>(loader, "LoadFileEnum")
    .value("LOAD_FIRST", f3d::loader::LoadFileEnum::LOAD_FIRST)
    .value("LOAD_PREVIOUS", f3d::loader::LoadFileEnum::LOAD_PREVIOUS)
    .value("LOAD_CURRENT", f3d::loader::LoadFileEnum::LOAD_CURRENT)
    .value("LOAD_NEXT", f3d::loader::LoadFileEnum::LOAD_NEXT)
    .value("LOAD_LAST", f3d::loader::LoadFileEnum::LOAD_LAST)
    .export_values();

  loader
    .def("addFiles", &f3d::loader::addFiles, "Add files to the loader", py::arg("list of files"),
      py::arg("recursive") = false)
    .def("addFile", &f3d::loader::addFile, "Add a file or directory to the loader", py::arg("path"),
      py::arg("recursive") = false)
    .def("loadFile", &f3d::loader::loadFile, "Load a specific file",
      py::arg("cursor") = f3d::loader::LoadFileEnum::LOAD_CURRENT)
    .def("getFiles", &f3d::loader::getFiles, "Get the list of files")
    .def("setCurrentFileIndex", &f3d::loader::setCurrentFileIndex, "Set the current file index")
    .def("getCurrentFileIndex", &f3d::loader::getCurrentFileIndex, "Get the current file index")
    .def(
      "getFileInfo", &f3d::loader::getFileInfo, "Get the file index, path, and information string");

  // f3d::camera
  py::class_<f3d::camera, std::unique_ptr<f3d::camera, py::nodelete> > camera(module, "camera");

  camera.def("setPosition", &f3d::camera::setPosition)
    .def("getPosition", py::overload_cast<>(&f3d::camera::getPosition))
    .def("getPosition", py::overload_cast<f3d::point3_t&>(&f3d::camera::getPosition))
    .def("setFocalPoint", &f3d::camera::setFocalPoint)
    .def("getFocalPoint", py::overload_cast<>(&f3d::camera::getFocalPoint))
    .def("getFocalPoint", py::overload_cast<f3d::point3_t&>(&f3d::camera::getFocalPoint))
    .def("setViewUp", &f3d::camera::setViewUp)
    .def("getViewUp", py::overload_cast<>(&f3d::camera::getViewUp))
    .def("getViewUp", py::overload_cast<f3d::vector3_t&>(&f3d::camera::getViewUp))
    .def("setViewAngle", &f3d::camera::setViewAngle)
    .def("getViewAngle", py::overload_cast<>(&f3d::camera::getViewAngle))
    .def("getViewAngle", py::overload_cast<f3d::angle_deg_t&>(&f3d::camera::getViewAngle))
    .def("dolly", &f3d::camera::dolly)
    .def("roll", &f3d::camera::roll)
    .def("azimuth", &f3d::camera::azimuth)
    .def("yaw", &f3d::camera::yaw)
    .def("elevation", &f3d::camera::elevation)
    .def("pitch", &f3d::camera::pitch)
    .def("setCurrentAsDefault", &f3d::camera::setCurrentAsDefault)
    .def("resetToDefault", &f3d::camera::resetToDefault)
    .def("resetToBounds", &f3d::camera::resetToBounds);

  // f3d::window
  py::class_<f3d::window, std::unique_ptr<f3d::window, py::nodelete> > window(module, "window");

  py::enum_<f3d::window::Type>(window, "Type")
    .value("NONE", f3d::window::Type::NONE)
    .value("NATIVE", f3d::window::Type::NATIVE)
    .value("NATIVE_OFFSCREEN", f3d::window::Type::NATIVE_OFFSCREEN)
    .value("EXTERNAL", f3d::window::Type::EXTERNAL)
    .export_values();

  window.def("render", &f3d::window::render, "Render the window")
    .def("renderToImage", &f3d::window::renderToImage, "Render the window to an image",
      py::arg("noBackground") = false)
    .def("setSize", &f3d::window::setSize, "Set the window size")
    .def("getWidth", &f3d::window::getWidth, "Get the window width")
    .def("getHeight", &f3d::window::getHeight, "Get the window height")
    .def("setIcon", &f3d::window::setIcon,
      "Set the icon of the window using a memory buffer representing a PNG file")
    .def("setWindowName", &f3d::window::setWindowName, "Set the window name")
    .def("getCamera", &f3d::window::getCamera, py::return_value_policy::reference)
    .def("getWorldFromDisplay", &f3d::window::getWorldFromDisplay,
      "Get world coordinate point from display coordinate")
    .def("getDisplayFromWorld", &f3d::window::getDisplayFromWorld,
      "Get display coordinate point from world coordinate");

  // f3d::engine
  py::class_<f3d::engine> engine(module, "engine");

  engine.def(py::init<f3d::window::Type>(), py::arg("windowType") = f3d::window::Type::NATIVE)
    .def("getInteractor", &f3d::engine::getInteractor, py::return_value_policy::reference)
    .def("getLoader", &f3d::engine::getLoader, py::return_value_policy::reference)
    .def("getOptions", &f3d::engine::getOptions, py::return_value_policy::reference)
    .def("setOptions", py::overload_cast<const f3d::options&>(&f3d::engine::setOptions))
    .def("getWindow", &f3d::engine::getWindow, py::return_value_policy::reference)
    .def_static("loadPlugin", &f3d::engine::loadPlugin, "Load a plugin")
    .def_static(
      "autoloadPlugins", &f3d::engine::autoloadPlugins, "Automatically load internal plugins");
}
