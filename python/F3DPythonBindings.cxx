#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "engine.h"
#include "image.h"
#include "interactor.h"
#include "loader.h"
#include "options.h"
#include "window.h"

namespace py = pybind11;

PYBIND11_MODULE(f3d, module)
{
  module.doc() = "f3d library bindings";

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
    .def("setData", &f3d::image::setData)
    .def("getData", &f3d::image::getData)
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
    .def("getAsDoubleVector", &f3d::options::getAsDoubleVector);

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
    .def("addFiles", &f3d::loader::addFiles, "Add files to the loader", py::arg("list of files"))
    .def("addFile", &f3d::loader::addFile, "Add a file or directory to the loader", py::arg("path"),
      py::arg("recursive") = false)
    .def("loadFile", &f3d::loader::loadFile, "Load a specific file",
      py::arg("cursor") = f3d::loader::LoadFileEnum::LOAD_CURRENT)
    .def("getFiles", &f3d::loader::getFiles, "Get the list of files")
    .def("setCurrentFileIndex", &f3d::loader::setCurrentFileIndex, "Set the current file index")
    .def("getCurrentFileIndex", &f3d::loader::getCurrentFileIndex, "Get the current file index")
    .def(
      "getFileInfo", &f3d::loader::getFileInfo, "Get the file index, path, and information string");

  // f3d::window
  py::class_<f3d::window, std::unique_ptr<f3d::window, py::nodelete> >(module, "window")
    .def("update", &f3d::window::update, "Update the window")
    .def("render", &f3d::window::render, "Render the window")
    .def("renderToImage", &f3d::window::renderToImage, "Render the window to an image",
      py::arg("noBackground") = false)
    .def("setIcon", &f3d::window::setIcon,
      "Set the icon of the window using a memory buffer representing a PNG file")
    .def("setWindowName", &f3d::window::setWindowName, "Set the window name");

  // f3d::engine
  py::class_<f3d::engine> engine(module, "engine");

  py::enum_<f3d::engine::Flags>(engine, "Flags", py::arithmetic())
    .value("FLAGS_NONE", f3d::engine::FLAGS_NONE)
    .value("CREATE_WINDOW", f3d::engine::CREATE_WINDOW)
    .value("CREATE_INTERACTOR", f3d::engine::CREATE_INTERACTOR)
    .value("WINDOW_OFFSCREEN", f3d::engine::WINDOW_OFFSCREEN)
    .export_values();

  engine.def(py::init<f3d::engine::flags_t>())
    .def("getInteractor", &f3d::engine::getInteractor, py::return_value_policy::reference)
    .def("getLoader", &f3d::engine::getLoader, py::return_value_policy::reference)
    .def("getOptions", &f3d::engine::getOptions, py::return_value_policy::reference)
    .def("setOptions", py::overload_cast<const f3d::options&>(&f3d::engine::setOptions))
    .def("getWindow", &f3d::engine::getWindow, py::return_value_policy::reference);
}
