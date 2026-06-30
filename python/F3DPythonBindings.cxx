#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "camera.h"
#include "engine.h"
#include "image.h"
#include "interactor.h"
#include "log.h"
#include "mesh_view.h"
#include "options.h"
#include "scene.h"
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

std::pair<size_t, f3d::mesh_view::data_array_t> fromBuffer(py::buffer buf)
{
  py::buffer_info info = buf.request();

  f3d::mesh_view::data_array_t dataArray;

  if (info.item_type_is_equivalent_to<uint8_t>())
  {
    dataArray.type = f3d::mesh_view::data_type::U8;
    dataArray.stride = info.strides[0] / sizeof(uint8_t);
  }
  else if (info.item_type_is_equivalent_to<int8_t>())
  {
    dataArray.type = f3d::mesh_view::data_type::I8;
    dataArray.stride = info.strides[0] / sizeof(int8_t);
  }
  else if (info.item_type_is_equivalent_to<uint16_t>())
  {
    dataArray.type = f3d::mesh_view::data_type::U16;
    dataArray.stride = info.strides[0] / sizeof(uint16_t);
  }
  else if (info.item_type_is_equivalent_to<int16_t>())
  {
    dataArray.type = f3d::mesh_view::data_type::I16;
    dataArray.stride = info.strides[0] / sizeof(int16_t);
  }
  else if (info.item_type_is_equivalent_to<uint32_t>())
  {
    dataArray.type = f3d::mesh_view::data_type::U32;
    dataArray.stride = info.strides[0] / sizeof(uint32_t);
  }
  else if (info.item_type_is_equivalent_to<int32_t>())
  {
    dataArray.type = f3d::mesh_view::data_type::I32;
    dataArray.stride = info.strides[0] / sizeof(int32_t);
  }
  else if (info.item_type_is_equivalent_to<float>())
  {
    dataArray.type = f3d::mesh_view::data_type::F32;
    dataArray.stride = info.strides[0] / sizeof(float);
  }
  else if (info.item_type_is_equivalent_to<double>())
  {
    dataArray.type = f3d::mesh_view::data_type::F64;
    dataArray.stride = info.strides[0] / sizeof(double);
  }
  else
  {
    throw std::runtime_error("Unsupported scalar type!");
  }

  if (info.ndim == 1)
  {
    dataArray.components = 1;
  }
  else if (info.ndim == 2)
  {
    dataArray.components = info.shape[1];
  }
  else
  {
    throw std::runtime_error("Incompatible buffer dimension!");
  }

  dataArray.data = info.ptr;

  return { info.shape[0], std::move(dataArray) };
}

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
    .def(py::init<const std::filesystem::path&>())
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
      [](const f3d::image& img, std::string key)
      {
        try
        {
          return img.getMetadata(std::move(key));
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
          // failed to set an `int`, try again as `double`
          if (std::holds_alternative<int>(value))
          {
            opts.set(key, static_cast<double>(std::get<int>(value)));
          }
          // failed to set a `vector<int>`, try again as `vector<double>`
          else if (std::holds_alternative<std::vector<int>>(value))
          {
            const std::vector<int>& ints = std::get<std::vector<int>>(value);
            const std::vector<double> doubles(ints.begin(), ints.end());
            opts.set(key, doubles);
          }
          // failed to set a `string`, parse and try again
          else if (std::holds_alternative<std::string>(value))
          {
            try
            {
              opts.setAsString(key, std::get<std::string>(value));
            }
            catch (const f3d::options::parsing_exception&)
            {
              throw py::value_error(std::get<std::string>(value));
            }
          }
          else
          {
            throw py::type_error(key);
          }
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

  py::enum_<f3d::utils::KnownFolder>(utils, "KnownFolder")
    .value("ROAMINGAPPDATA", f3d::utils::KnownFolder::ROAMINGAPPDATA)
    .value("LOCALAPPDATA", f3d::utils::KnownFolder::LOCALAPPDATA)
    .value("PICTURES", f3d::utils::KnownFolder::PICTURES)
    .export_values();

  utils //
    .def_static("text_distance", &f3d::utils::textDistance)
    .def_static("collapse_path", &f3d::utils::collapsePath)
    .def_static("tokenize", &f3d::utils::tokenize, py::arg("str"), py::arg("keep_comments") = true)
    .def_static(
      "glob_to_regex", &f3d::utils::globToRegex, py::arg("glob"), py::arg("path_separator") = '/')
    .def_static("get_dpi_scale", &f3d::utils::getDPIScale)
    .def_static("get_env", &f3d::utils::getEnv)
    .def_static("get_known_folder", &f3d::utils::getKnownFolder);

  // f3d::interactor
  py::class_<f3d::interaction_bind_t> interaction_bind(module, "InteractionBind");

  py::enum_<f3d::interaction_bind_t::ModifierKeys>(interaction_bind, "ModifierKeys")
    .value("ANY", f3d::interaction_bind_t::ModifierKeys::ANY)
    .value("NONE", f3d::interaction_bind_t::ModifierKeys::NONE)
    .value("CTRL", f3d::interaction_bind_t::ModifierKeys::CTRL)
    .value("SHIFT", f3d::interaction_bind_t::ModifierKeys::SHIFT)
    .value("CTRL_SHIFT", f3d::interaction_bind_t::ModifierKeys::CTRL_SHIFT)
    .export_values();
  interaction_bind.def(py::init<const f3d::interaction_bind_t::ModifierKeys&, const std::string&>())
    .def_readwrite("mod", &f3d::interaction_bind_t::mod)
    .def_readwrite("inter", &f3d::interaction_bind_t::inter)
    .def("format", &f3d::interaction_bind_t::format);

  py::class_<f3d::interactor_state_t> interactor_state(module, "InteractorState");

  interactor_state.def(py::init<>())
    .def_readonly("animation_time", &f3d::interactor_state_t::animationTime);

  py::class_<f3d::interactor, std::unique_ptr<f3d::interactor, py::nodelete>> interactor(
    module, "Interactor");

  py::enum_<f3d::interactor::BindingType>(interactor, "BindingType")
    .value("CYCLIC", f3d::interactor::BindingType::CYCLIC)
    .value("NUMERICAL", f3d::interactor::BindingType::NUMERICAL)
    .value("TOGGLE", f3d::interactor::BindingType::TOGGLE)
    .value("OTHER", f3d::interactor::BindingType::OTHER)
    .export_values();
  py::enum_<f3d::interactor::MouseButton>(interactor, "MouseButton")
    .value("LEFT", f3d::interactor::MouseButton::LEFT)
    .value("MIDDLE", f3d::interactor::MouseButton::MIDDLE)
    .value("RIGHT", f3d::interactor::MouseButton::RIGHT)
    .export_values();

  py::enum_<f3d::interactor::WheelDirection>(interactor, "WheelDirection")
    .value("FORWARD", f3d::interactor::WheelDirection::FORWARD)
    .value("BACKWARD", f3d::interactor::WheelDirection::BACKWARD)
    .value("LEFT", f3d::interactor::WheelDirection::LEFT)
    .value("RIGHT", f3d::interactor::WheelDirection::RIGHT)
    .export_values();

  py::enum_<f3d::interactor::InputAction>(interactor, "InputAction")
    .value("PRESS", f3d::interactor::InputAction::PRESS)
    .value("RELEASE", f3d::interactor::InputAction::RELEASE)
    .export_values();

  py::enum_<f3d::interactor::InputModifier>(interactor, "InputModifier")
    .value("NONE", f3d::interactor::InputModifier::NONE)
    .value("CTRL", f3d::interactor::InputModifier::CTRL)
    .value("SHIFT", f3d::interactor::InputModifier::SHIFT)
    .value("CTRL_SHIFT", f3d::interactor::InputModifier::CTRL_SHIFT)
    .export_values();

  py::enum_<f3d::interactor::AnimationDirection>(interactor, "AnimationDirection")
    .value("FORWARD", f3d::interactor::AnimationDirection::FORWARD)
    .value("BACKWARD", f3d::interactor::AnimationDirection::BACKWARD)
    .export_values();

  interactor //
    .def("toggle_animation", &f3d::interactor::toggleAnimation, "Toggle the animation",
      py::arg("direction") = f3d::interactor::AnimationDirection::FORWARD)
    .def("start_animation", &f3d::interactor::startAnimation, "Start the animation",
      py::arg("direction") = f3d::interactor::AnimationDirection::FORWARD)
    .def("stop_animation", &f3d::interactor::stopAnimation, "Stop the animation")
    .def("is_playing_animation", &f3d::interactor::isPlayingAnimation,
      "Returns True if the animation is currently started")
    .def("get_animation_direction", &f3d::interactor::getAnimationDirection,
      "Returns the current animation direction")
    .def("enable_camera_movement", &f3d::interactor::enableCameraMovement,
      "Enable the camera interaction")
    .def("disable_camera_movement", &f3d::interactor::disableCameraMovement,
      "Disable the camera interaction")
    .def("set_event_loop_user_callback", &f3d::interactor::setEventLoopUserCallback,
      "Set the user callback of the event loop", py::arg("user_callback") = nullptr)
    .def("trigger_mod_update", &f3d::interactor::triggerModUpdate, "Trigger a key modifier update")
    .def("trigger_mouse_button", &f3d::interactor::triggerMouseButton, "Trigger a mouse button")
    .def(
      "trigger_mouse_position", &f3d::interactor::triggerMousePosition, "Trigger a mouse position")
    .def("trigger_mouse_wheel", &f3d::interactor::triggerMouseWheel, "Trigger a mouse wheel")
    .def("trigger_keyboard_key", &f3d::interactor::triggerKeyboardKey, "Trigger a keyboard input")
    .def("trigger_text_character", &f3d::interactor::triggerTextCharacter,
      "Trigger a text character input")
    .def(
      "trigger_event_loop", &f3d::interactor::triggerEventLoop, "Manually trigger the event loop.")
    .def("play_interaction", &f3d::interactor::playInteraction, "Play an interaction file")
    .def("record_interaction", &f3d::interactor::recordInteraction, "Record an interaction file")
    .def("start", &f3d::interactor::start, "Start the interactor and the event loop",
      py::arg("delta_time") = 1.0 / 30)
    .def("stop", &f3d::interactor::stop, "Stop the interactor and the event loop")
    .def(
      "request_render", &f3d::interactor::requestRender, "Request a render on the next event loop")
    .def("request_stop", &f3d::interactor::requestStop, "Stop on the next event loop")
    .def("init_commands", &f3d::interactor::initCommands,
      "Remove all commands and add all default command callbacks")
    .def("add_command", &f3d::interactor::addCommand, "Add a command", py::arg("action"),
      py::arg("callback"), py::arg("doc") = std::nullopt, py::arg("completionCallback") = nullptr)
    .def("remove_command", &f3d::interactor::removeCommand, "Remove a command")
    .def("get_command_actions", &f3d::interactor::getCommandActions, "Get all command actions")
    .def("trigger_command", &f3d::interactor::triggerCommand, "Trigger a command",
      py::arg("command"), py::arg("keep_comments") = true)
    .def("init_bindings", &f3d::interactor::initBindings,
      "Remove all bindings and add default bindings")
    .def("remove_binding", &f3d::interactor::removeBinding, "Remove interaction commands")
    .def("get_bind_groups", &f3d::interactor::getBindGroups)
    .def("get_binds_for_group", &f3d::interactor::getBindsForGroup)
    .def("get_binds", &f3d::interactor::getBinds)
    .def("get_binding_documentation", &f3d::interactor::getBindingDocumentation)
    .def("get_binding_type", &f3d::interactor::getBindingType)
    .def("trigger_notification", &f3d::interactor::triggerNotification,
      "Trigger a single text line notification at the bottom left of viewport", py::arg("desc"),
      py::arg("value") = "", py::arg("duration") = 3.0);

  interactor
    .def("add_binding",
      py::overload_cast<const f3d::interaction_bind_t&, std::string, std::string,
        std::function<std::pair<std::string, std::string>()>, f3d::interactor::BindingType, bool>(
        &f3d::interactor::addBinding),
      "Add a binding command", py::arg("bind"), py::arg("command"), py::arg("group"),
      py::arg("documentationCallback") = nullptr,
      py::arg("type") = f3d::interactor::BindingType::OTHER, py::arg("notify") = true)
    .def("add_binding",
      py::overload_cast<const f3d::interaction_bind_t&, std::vector<std::string>, std::string,
        std::function<std::pair<std::string, std::string>()>, f3d::interactor::BindingType, bool>(
        &f3d::interactor::addBinding),
      "Add binding commands", py::arg("bind"), py::arg("command"), py::arg("group"),
      py::arg("documentationCallback") = nullptr,
      py::arg("type") = f3d::interactor::BindingType::OTHER, py::arg("notify") = true);

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

  // f3d::mesh_view
  py::class_<f3d::mesh_view::memory_view_t>(module, "MeshMemoryView", py::buffer_protocol())
    .def(py::init<>())
    .def_property("points", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, dataArray] = fromBuffer(b);

        self.pointCount = count;
        bool timeDependent = self.points.timeDependent;
        self.points = std::move(dataArray);
        self.points.timeDependent = timeDependent;
      })
    .def_property("points_time_dependent", nullptr,
      [](f3d::mesh_view::memory_view_t& self, bool timeDependent)
      { self.points.timeDependent = timeDependent; })
    .def_property("normals", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, dataArray] = fromBuffer(b);

        if (count != self.pointCount)
        {
          throw std::runtime_error("Incompatible buffer shape: point count does not match!");
        }

        bool timeDependent = self.normals.timeDependent;
        self.normals = std::move(dataArray);
        self.normals.timeDependent = timeDependent;
      })
    .def_property("normals_time_dependent", nullptr,
      [](f3d::mesh_view::memory_view_t& self, bool timeDependent)
      { self.normals.timeDependent = timeDependent; })
    .def_property("texture_coordinates", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, dataArray] = fromBuffer(b);

        if (count != self.pointCount)
        {
          throw std::runtime_error("Incompatible buffer shape: point count does not match!");
        }

        bool timeDependent = self.textureCoordinates.timeDependent;
        self.textureCoordinates = std::move(dataArray);
        self.textureCoordinates.timeDependent = timeDependent;
      })
    .def_property("texture_coordinates_time_dependent", nullptr,
      [](f3d::mesh_view::memory_view_t& self, bool timeDependent)
      { self.textureCoordinates.timeDependent = timeDependent; })
    .def_property("vertices_offsets", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, array] = fromBuffer(b);
        self.vertices.offsetCount = count;
        bool timeDependent = self.vertices.offsets.timeDependent;
        self.vertices.offsets = std::move(array);
        self.vertices.offsets.timeDependent = timeDependent;
      })
    .def_property("vertices_indices", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, array] = fromBuffer(b);
        self.vertices.indexCount = count;
        bool timeDependent = self.vertices.indices.timeDependent;
        self.vertices.indices = std::move(array);
        self.vertices.indices.timeDependent = timeDependent;
      })
    .def_property("vertices_time_dependent", nullptr,
      [](f3d::mesh_view::memory_view_t& self, bool timeDependent)
      {
        self.vertices.indices.timeDependent = timeDependent;
        self.vertices.offsets.timeDependent = timeDependent;
      })
    .def_property("lines_offsets", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, array] = fromBuffer(b);
        self.lines.offsetCount = count;
        bool timeDependent = self.lines.offsets.timeDependent;
        self.lines.offsets = std::move(array);
        self.lines.offsets.timeDependent = timeDependent;
      })
    .def_property("lines_indices", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, array] = fromBuffer(b);
        self.lines.indexCount = count;
        bool timeDependent = self.lines.indices.timeDependent;
        self.lines.indices = std::move(array);
        self.lines.indices.timeDependent = timeDependent;
      })
    .def_property("lines_time_dependent", nullptr,
      [](f3d::mesh_view::memory_view_t& self, bool timeDependent)
      {
        self.lines.indices.timeDependent = timeDependent;
        self.lines.offsets.timeDependent = timeDependent;
      })
    .def_property("polygons_offsets", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, array] = fromBuffer(b);
        self.polygons.offsetCount = count;
        bool timeDependent = self.polygons.offsets.timeDependent;
        self.polygons.offsets = std::move(array);
        self.polygons.offsets.timeDependent = timeDependent;
      })
    .def_property("polygons_indices", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::buffer b)
      {
        auto [count, array] = fromBuffer(b);
        self.polygons.indexCount = count;
        bool timeDependent = self.polygons.indices.timeDependent;
        self.polygons.indices = std::move(array);
        self.polygons.indices.timeDependent = timeDependent;
      })
    .def_property("polygons_time_dependent", nullptr,
      [](f3d::mesh_view::memory_view_t& self, bool timeDependent)
      {
        self.polygons.indices.timeDependent = timeDependent;
        self.polygons.offsets.timeDependent = timeDependent;
      })
    .def_property("point_scalars", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::dict d)
      {
        self.pointScalars.clear();

        for (auto item : d)
        {
          f3d::mesh_view::data_array_t dataArray =
            fromBuffer(py::cast<py::buffer>(item.second)).second;
          dataArray.name = py::cast<std::string>(item.first);
          self.pointScalars.emplace_back(std::move(dataArray));
        }
      })
    .def("set_point_scalars_time_dependent",
      [](f3d::mesh_view::memory_view_t& self, const std::string& name, bool timeDependent)
      {
        auto it = std::ranges::find_if(self.pointScalars,
          [&name](const f3d::mesh_view::data_array_t& array) { return array.name == name; });
        if (it == self.pointScalars.end())
        {
          throw std::runtime_error("No point scalar with name " + name);
        }
        it->timeDependent = timeDependent;
      })
    .def_property("cell_scalars", nullptr,
      [](f3d::mesh_view::memory_view_t& self, py::dict d)
      {
        self.cellScalars.clear();

        for (auto item : d)
        {
          f3d::mesh_view::data_array_t dataArray =
            fromBuffer(py::cast<py::buffer>(item.second)).second;
          dataArray.name = py::cast<std::string>(item.first);
          self.cellScalars.emplace_back(std::move(dataArray));
        }
      })
    .def("set_cell_scalars_time_dependent",
      [](f3d::mesh_view::memory_view_t& self, const std::string& name, bool timeDependent)
      {
        auto it = std::ranges::find_if(self.cellScalars,
          [&name](const f3d::mesh_view::data_array_t& array) { return array.name == name; });
        if (it == self.cellScalars.end())
        {
          throw std::runtime_error("No cell scalar with name " + name);
        }
        it->timeDependent = timeDependent;
      })
    // Optional in-memory base-color (albedo) texture, sampled via texture_coordinates. Assign an
    // f3d.Image (a BYTE image with 3 or 4 channels); leave default (empty) for no texture. When
    // base_color_texture_emissive is true the same image is also installed as the emissive map
    // (flat / full-strength display).
    .def_readwrite("base_color_texture", &f3d::mesh_view::memory_view_t::baseColorTexture)
    .def_readwrite("base_color_texture_emissive",
                   &f3d::mesh_view::memory_view_t::baseColorTextureEmissive);

  class PyMesh
    : public f3d::mesh_view
    , public py::trampoline_self_life_support
  {
  public:
    std::array<double, 2> getTimeRange() const override
    {
      py::gil_scoped_acquire gil;
      py::function fn = py::get_override(this, "get_time_range");
      if (fn)
      {
        return fn().cast<std::array<double, 2>>();
      }
      return f3d::mesh_view::getTimeRange();
    }

    std::string getName() const override
    {
      py::gil_scoped_acquire gil;
      py::function fn = py::get_override(this, "get_name");
      if (fn)
      {
        return fn().cast<std::string>();
      }
      return f3d::mesh_view::getName();
    }

    memory_view_t getMemoryView(double time) const override
    {
      py::gil_scoped_acquire gil;
      py::function fn = py::get_override(this, "get_memory_view");
      if (fn)
      {
        return fn(time).cast<memory_view_t>();
      }
      throw std::domain_error("Tried to call pure virtual function getMemoryView");
    }
  };

  py::class_<f3d::mesh_view, PyMesh, py::smart_holder>(module, "MeshView")
    .def(py::init<>())
    .def("get_time_range", &f3d::mesh_view::getTimeRange)
    .def("get_name", &f3d::mesh_view::getName)
    .def("get_memory_view", &f3d::mesh_view::getMemoryView);

  // f3d::color_t
  py::class_<f3d::color_t>(module, "Color")
    .def(py::init<>())
    .def(py::init<double, double, double>(), py::arg("r"), py::arg("g"), py::arg("b"))
    .def_property(
      "r", [](const f3d::color_t& color) { return color.r(); },
      [](f3d::color_t& color, double value) { color[0] = value; })
    .def_property(
      "g", [](const f3d::color_t& color) { return color.g(); },
      [](f3d::color_t& color, double value) { color[1] = value; })
    .def_property(
      "b", [](const f3d::color_t& color) { return color.b(); },
      [](f3d::color_t& color, double value) { color[2] = value; })
    .def(
      "to_tuple",
      [](const f3d::color_t& color) { return std::make_tuple(color.r(), color.g(), color.b()); },
      "Convert color to a tuple of (r, g, b)")
    .def(
      "from_tuple",
      [](f3d::color_t& color, const std::tuple<double, double, double>& tuple)
      {
        color[0] = std::get<0>(tuple);
        color[1] = std::get<1>(tuple);
        color[2] = std::get<2>(tuple);
      },
      "Set color from a tuple of (r, g, b)");

  py::enum_<f3d::light_type>(module, "LightType")
    .value("HEADLIGHT", f3d::light_type::HEADLIGHT)
    .value("CAMERA_LIGHT", f3d::light_type::CAMERA_LIGHT)
    .value("SCENE_LIGHT", f3d::light_type::SCENE_LIGHT)
    .export_values();

  // f3d::light_state_t
  py::class_<f3d::light_state_t>(module, "LightState")
    .def(py::init<>())
    .def(py::init<const f3d::light_type&, const f3d::point3_t&, const f3d::color_t&,
           const f3d::vector3_t&, const bool&, const double&, const bool&>(),
      py::arg("type") = f3d::light_type::SCENE_LIGHT,
      py::arg("position") = f3d::point3_t({ 0.0, 0.0, 0.0 }),
      py::arg_v("color", f3d::color_t({ 1.0, 1.0, 1.0 }), "Color(1.0, 1.0, 1.0)"),
      py::arg("direction") = f3d::vector3_t({ 1.0, 0.0, 0.0 }), py::arg("positional_light") = false,
      py::arg("intensity") = 1.0, py::arg("switch_state") = true)
    .def_readwrite("type", &f3d::light_state_t::type)
    .def_readwrite("color", &f3d::light_state_t::color)
    .def_readwrite("position", &f3d::light_state_t::position)
    .def_readwrite("direction", &f3d::light_state_t::direction)
    .def_readwrite("positional_light", &f3d::light_state_t::positionalLight)
    .def_readwrite("intensity", &f3d::light_state_t::intensity)
    .def_readwrite("switch_state", &f3d::light_state_t::switchState);

  // f3d::scene
  py::class_<f3d::scene, std::unique_ptr<f3d::scene, py::nodelete>> scene(module, "Scene");
  scene //
    .def("supports", &f3d::scene::supports)
    .def("clear", &f3d::scene::clear)
    .def("add", py::overload_cast<const std::filesystem::path&>(&f3d::scene::add),
      "Add a file the scene", py::arg("file_path"))
    .def("add", py::overload_cast<const std::vector<std::filesystem::path>&>(&f3d::scene::add),
      "Add multiple filepaths to the scene", py::arg("file_path_vector"))
    .def("add", py::overload_cast<const std::vector<std::string>&>(&f3d::scene::add),
      "Add multiple filenames to the scene", py::arg("file_name_vector"))
    .def("add", py::overload_cast<const f3d::mesh_t&>(&f3d::scene::add),
      "Add a surfacic mesh from memory into the scene", py::arg("mesh"))
    .def("add", py::overload_cast<std::shared_ptr<f3d::mesh_view>>(&f3d::scene::add),
      "Add a surfacic mesh view from memory into the scene", py::arg("mesh"))
    .def(
      "add",
      [](f3d::scene& scene, py::bytes buffer, std::size_t size)
      {
        PyErr_WarnEx(
          PyExc_DeprecationWarning, "add(buffer, size) is deprecated, use add(buffer) instead.", 1);
        std::string_view sv(buffer);
        scene.add(reinterpret_cast<const std::byte*>(sv.data()), size);
      },
      "Add a memory buffer containing a file the scene", py::arg("buffer"), py::arg("size"))
    .def(
      "add",
      [](f3d::scene& scene, py::bytes buffer)
      {
        std::string_view sv(buffer);
        scene.add(reinterpret_cast<const std::byte*>(sv.data()), sv.size());
      },
      "Add a memory buffer containing a file the scene", py::arg("buffer"), py::prepend())
    .def("load_animation_time", &f3d::scene::loadAnimationTime)
    .def("animation_time_range", &f3d::scene::animationTimeRange)
    .def("get_animation_keyframes", &f3d::scene::getAnimationKeyFrames)
    .def("available_animations", &f3d::scene::availableAnimations)
    .def("get_animation_name", &f3d::scene::getAnimationName, py::arg("index") = -1,
      "Returns the animation at an index (defaults to current)")
    .def("get_animation_names", &f3d::scene::getAnimationNames, "Returns all animation names")
    .def("add_light", &f3d::scene::addLight, "Add a light to the scene", py::arg("light_state"))
    .def(
      "remove_light", &f3d::scene::removeLight, "Remove a light from the scene", py::arg("index"))
    .def("update_light", &f3d::scene::updateLight, "Update a light in the scene", py::arg("index"),
      py::arg("light_state"))
    .def("get_light", &f3d::scene::getLight, "Get a light from the scene", py::arg("index"))
    .def("get_light_count", &f3d::scene::getLightCount, "Get the number of lights in the scene")
    .def("remove_all_lights", &f3d::scene::removeAllLights, "Remove all lights from the scene");

  // f3d::camera_state_t
  py::class_<f3d::camera_state_t>(module, "CameraState")
    .def(py::init<>())
    .def(py::init<const f3d::point3_t&, const f3d::point3_t&, const f3d::vector3_t&,
           const f3d::angle_deg_t&>(),
      py::arg("position") = f3d::point3_t({ 0., 0., 1. }),
      py::arg("focal_point") = f3d::point3_t({ 0., 0., 0. }),
      py::arg("view_up") = f3d::vector3_t({ 0, 1, 0 }), py::arg("view_angle") = 30.)
    .def_readwrite("position", &f3d::camera_state_t::position)
    .def_readwrite("focal_point", &f3d::camera_state_t::focalPoint)
    .def_readwrite("view_up", &f3d::camera_state_t::viewUp)
    .def_readwrite("view_angle", &f3d::camera_state_t::viewAngle);

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

  // f3d::window
  py::class_<f3d::window, std::unique_ptr<f3d::window, py::nodelete>> window(module, "Window");

  py::enum_<f3d::window::Type>(window, "Type")
    .value("NONE", f3d::window::Type::NONE)
    .value("EXTERNAL", f3d::window::Type::EXTERNAL)
    .value("GLX", f3d::window::Type::GLX)
    .value("WGL", f3d::window::Type::WGL)
    .value("COCOA", f3d::window::Type::COCOA)
    .value("EGL", f3d::window::Type::EGL)
    .value("OSMESA", f3d::window::Type::OSMESA)
    .value("UNKNOWN", f3d::window::Type::UNKNOWN)
    .export_values();

  window //
    .def_property_readonly("type", &f3d::window::getType)
    .def_property_readonly("offscreen", &f3d::window::isOffscreen)
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

  // libInformation
  py::class_<f3d::engine::libInformation>(module, "LibInformation")
    .def_readonly("version", &f3d::engine::libInformation::Version)
    .def_readonly("version_full", &f3d::engine::libInformation::VersionFull)
    .def_readonly("build_date", &f3d::engine::libInformation::BuildDate)
    .def_readonly("build_system", &f3d::engine::libInformation::BuildSystem)
    .def_readonly("compiler", &f3d::engine::libInformation::Compiler)
    .def_readonly("modules", &f3d::engine::libInformation::Modules)
    .def_readonly("vtk_version", &f3d::engine::libInformation::VTKVersion)
    .def_readonly("copyrights", &f3d::engine::libInformation::Copyrights)
    .def_readonly("license", &f3d::engine::libInformation::License);

  // readerInformation
  py::class_<f3d::engine::readerInformation>(module, "ReaderInformation")
    .def_readonly("name", &f3d::engine::readerInformation::Name)
    .def_readonly("description", &f3d::engine::readerInformation::Description)
    .def_readonly("extensions", &f3d::engine::readerInformation::Extensions)
    .def_readonly("mime_types", &f3d::engine::readerInformation::MimeTypes)
    .def_readonly("plugin_name", &f3d::engine::readerInformation::PluginName)
    .def_readonly("has_scene_reader", &f3d::engine::readerInformation::HasSceneReader)
    .def_readonly("has_geometry_reader", &f3d::engine::readerInformation::HasGeometryReader);

  // f3d::engine
  py::class_<f3d::engine> engine(module, "Engine");

  engine //
    .def_static("create", &f3d::engine::create, "Create an engine with a automatic window",
      py::arg("offscreen") = false)
    .def_static("create_none", &f3d::engine::createNone, "Create an engine with no window")
    .def_static(
      "create_glx", &f3d::engine::createGLX, "Create an engine with an GLX window (Linux only)")
    .def_static(
      "create_wgl", &f3d::engine::createWGL, "Create an engine with an WGL window (Windows only)")
    .def_static("create_egl", &f3d::engine::createEGL,
      "Create an engine with an EGL window (Windows/Linux only)")
    .def_static("create_osmesa", &f3d::engine::createOSMesa,
      "Create an engine with an OSMesa window (Windows/Linux only)")
    .def_static(
      "create_external",
      [](py::object py_get_proc)
      {
        f3d::context::function func = [py_get_proc](const char* name) -> f3d::context::fptr
        {
          uintptr_t addr = py::int_(py_get_proc(py::bytes(name)));
          return reinterpret_cast<f3d::context::fptr>(addr);
        };
        return f3d::engine::createExternal(func);
      },
      py::arg("get_proc_address"),
      "Create an engine with an existing context via a get_proc_address callback")
    .def_static("create_external_glx", &f3d::engine::createExternalGLX,
      "Create an engine with an existing GLX context (Linux only)")
    .def_static("create_external_wgl", &f3d::engine::createExternalWGL,
      "Create an engine with an existing WGL context (Windows only)")
    .def_static("create_external_cocoa", &f3d::engine::createExternalCOCOA,
      "Create an engine with an existing COCOA context (macOS only)")
    .def_static("create_external_egl", &f3d::engine::createExternalEGL,
      "Create an engine with an existing EGL context (Windows/Linux only)")
    .def_static("create_external_osmesa", &f3d::engine::createExternalOSMesa,
      "Create an engine with an existing OSMesa context (Windows/Linux only)")
    .def("set_cache_path", &f3d::engine::setCachePath, "Set the cache path directory")
    .def_property("options", &f3d::engine::getOptions,
      py::overload_cast<const f3d::options&>(&f3d::engine::setOptions),
      py::return_value_policy::reference)
    .def_property_readonly("window", &f3d::engine::getWindow, py::return_value_policy::reference)
    .def_property_readonly("scene", &f3d::engine::getScene, py::return_value_policy::reference)
    .def_property_readonly(
      "interactor", &f3d::engine::getInteractor, py::return_value_policy::reference)
    .def_static("load_plugin", &f3d::engine::loadPlugin, "Load a plugin")
    .def_static(
      "autoload_plugins", &f3d::engine::autoloadPlugins, "Automatically load internal plugins")
    .def_static("get_plugins_list", &f3d::engine::getPluginsList)
    .def_static("get_lib_info", &f3d::engine::getLibInfo, py::return_value_policy::reference)
    .def_static("get_readers_info", &f3d::engine::getReadersInfo)
    .def_static("get_rendering_backend_list", &f3d::engine::getRenderingBackendList)
    .def_static("set_reader_option",
      [](const std::string& name, const std::string& value)
      {
        try
        {
          f3d::engine::setReaderOption(name, value);
        }
        catch (const f3d::options::inexistent_exception&)
        {
          throw py::key_error(name);
        }
      })
    .def_static("get_all_reader_option_names", &f3d::engine::getAllReaderOptionNames);

  // f3d::log
  py::class_<f3d::log> log(module, "Log");

  py::enum_<f3d::log::VerboseLevel>(log, "VerboseLevel")
    .value("DEBUG", f3d::log::VerboseLevel::DEBUG)
    .value("INFO", f3d::log::VerboseLevel::INFO)
    .value("WARN", f3d::log::VerboseLevel::WARN)
    .value("ERROR", f3d::log::VerboseLevel::ERROR)
    .value("QUIET", f3d::log::VerboseLevel::QUIET)
    .export_values();

  auto forwardWrapper = [](f3d::log::forward_fn_t callback) { f3d::log::forward(callback); };

  module.add_object("forwardcleanup",
    py::capsule(&forwardWrapper, nullptr, [](PyObject*) { f3d::log::forward(nullptr); }));

  log //
    .def_static("set_verbose_level", &f3d::log::setVerboseLevel, py::arg("level"),
      py::arg("force_std_err") = false)
    .def_static("get_verbose_level", &f3d::log::getVerboseLevel)
    .def_static("set_use_coloring", &f3d::log::setUseColoring)
    .def_static("print", [](f3d::log::VerboseLevel& level, const std::string& message)
      { f3d::log::print(level, message); })
    .def_static("forward", forwardWrapper, py::arg("callback"));
}
