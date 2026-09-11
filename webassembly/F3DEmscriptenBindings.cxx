#include <emscripten/bind.h>

#include <array>
#include <memory>
#include <optional>
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

struct wasm_mesh_view : public f3d::mesh_view
{
  // not time support with wasm
  std::array<double, 2> getTimeRange() const override
  {
    return { 0.0, 0.0 };
  }

  std::string getName() const override
  {
    return this->Name;
  }

  memory_view_t getMemoryView(double) const override
  {
    memory_view_t view;

    view.pointCount = this->PointCount;

    view.points.name = "points";
    view.points.type = data_type::F32;
    view.points.data = this->Points.empty() ? nullptr : this->Points.data();
    view.points.components = 3;
    view.points.stride = 3;
    view.points.timeDependent = false;

    view.normals.name = "normals";
    view.normals.type = data_type::F32;
    view.normals.data = this->Normals.empty() ? nullptr : this->Normals.data();
    view.normals.components = 3;
    view.normals.stride = 3;
    view.normals.timeDependent = false;

    view.textureCoordinates.name = "textureCoordinates";
    view.textureCoordinates.type = data_type::F32;
    view.textureCoordinates.data =
      this->TextureCoordinates.empty() ? nullptr : this->TextureCoordinates.data();
    view.textureCoordinates.components = 2;
    view.textureCoordinates.stride = 2;
    view.textureCoordinates.timeDependent = false;

    view.polygons.offsetCount = this->PolygonOffsets.empty() ? 1 : this->PolygonOffsets.size();
    view.polygons.offsets.name = "polygonOffsets";
    view.polygons.offsets.type = data_type::U32;
    view.polygons.offsets.data =
      this->PolygonOffsets.empty() ? nullptr : this->PolygonOffsets.data();
    view.polygons.offsets.components = 1;
    view.polygons.offsets.stride = 1;
    view.polygons.offsets.timeDependent = false;

    view.polygons.indexCount = this->PolygonIndices.size();
    view.polygons.indices.name = "polygonIndices";
    view.polygons.indices.type = data_type::U32;
    view.polygons.indices.data =
      this->PolygonIndices.empty() ? nullptr : this->PolygonIndices.data();
    view.polygons.indices.components = 1;
    view.polygons.indices.stride = 1;
    view.polygons.indices.timeDependent = false;

    return view;
  }

  std::string Name;
  size_t PointCount = 0;
  std::vector<float> Points;
  std::vector<float> Normals;
  std::vector<float> TextureCoordinates;
  std::vector<unsigned int> PolygonOffsets;
  std::vector<unsigned int> PolygonIndices;
};

EMSCRIPTEN_BINDINGS(f3d)
{
  // types
  emscripten::value_array<f3d::point3_t>("Point3")
    .element(emscripten::index<0>())
    .element(emscripten::index<1>())
    .element(emscripten::index<2>());

  emscripten::value_array<f3d::vector3_t>("Vector3")
    .element(emscripten::index<0>())
    .element(emscripten::index<1>())
    .element(emscripten::index<2>());

  emscripten::class_<f3d::color_t>("Color")
    .constructor<double, double, double>()
    .property("r", &f3d::color_t::r)
    .property("g", &f3d::color_t::g)
    .property("b", &f3d::color_t::b);

  emscripten::enum_<f3d::light_type>("LightType")
    .value("HEADLIGHT", f3d::light_type::HEADLIGHT)
    .value("CAMERA_LIGHT", f3d::light_type::CAMERA_LIGHT)
    .value("SCENE_LIGHT", f3d::light_type::SCENE_LIGHT);

  emscripten::class_<f3d::light_state_t>("LightState")
    .constructor<>()
    .property("type", &f3d::light_state_t::type)
    .property("position", &f3d::light_state_t::position)
    .property("color", &f3d::light_state_t::color)
    .property("direction", &f3d::light_state_t::direction)
    .property("positionalLight", &f3d::light_state_t::positionalLight)
    .property("intensity", &f3d::light_state_t::intensity)
    .property("switchState", &f3d::light_state_t::switchState);

  emscripten::class_<f3d::mesh_t>("Mesh")
    .constructor<>()
    .property(
      "points",
      +[](const f3d::mesh_t& mesh) -> emscripten::val {
        return emscripten::val(
          emscripten::typed_memory_view(mesh.points.size(), mesh.points.data()));
      },
      +[](f3d::mesh_t& mesh, emscripten::val jsArray)
      { mesh.points = emscripten::convertJSArrayToNumberVector<float>(jsArray); })
    .property(
      "normals",
      +[](const f3d::mesh_t& mesh) -> emscripten::val
      {
        return emscripten::val(
          emscripten::typed_memory_view(mesh.normals.size(), mesh.normals.data()));
      },
      +[](f3d::mesh_t& mesh, emscripten::val jsArray)
      { mesh.normals = emscripten::convertJSArrayToNumberVector<float>(jsArray); })
    .property(
      "textureCoordinates",
      +[](const f3d::mesh_t& mesh) -> emscripten::val
      {
        return emscripten::val(emscripten::typed_memory_view(
          mesh.texture_coordinates.size(), mesh.texture_coordinates.data()));
      },
      +[](f3d::mesh_t& mesh, emscripten::val jsArray)
      { mesh.texture_coordinates = emscripten::convertJSArrayToNumberVector<float>(jsArray); })
    .property(
      "faceSides",
      +[](const f3d::mesh_t& mesh) -> emscripten::val
      {
        return emscripten::val(
          emscripten::typed_memory_view(mesh.face_sides.size(), mesh.face_sides.data()));
      },
      +[](f3d::mesh_t& mesh, emscripten::val jsArray)
      { mesh.face_sides = emscripten::convertJSArrayToNumberVector<unsigned int>(jsArray); })
    .property(
      "faceIndices",
      +[](const f3d::mesh_t& mesh) -> emscripten::val
      {
        return emscripten::val(
          emscripten::typed_memory_view(mesh.face_indices.size(), mesh.face_indices.data()));
      },
      +[](f3d::mesh_t& mesh, emscripten::val jsArray)
      { mesh.face_indices = emscripten::convertJSArrayToNumberVector<unsigned int>(jsArray); });

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
      "getRangeDomainAsNumber",
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
      "getEnumDomainAsString",
      +[](const f3d::options& o, const std::string& name) -> emscripten::val
      {
        // Only string is supported for now
        f3d::options::DomainEnum<f3d::option_variant_t> domain = o.getEnumDomain(name);
        std::vector<std::string> enumeration(domain.enumeration.size());
        std::transform(domain.enumeration.begin(), domain.enumeration.end(), enumeration.begin(),
          [](const auto& value) { return std::get<std::string>(value); });
        return containerToJSArray(enumeration);
      })
    .function(
      "getIndexDomain",
      +[](const f3d::options& o, const std::string& name) -> emscripten::val
      {
        f3d::options::DomainIndex domain = o.getIndexDomain(name);
        if (domain.max.has_value())
        {
          return emscripten::val(domain.max.value());
        }
        else
        {
          return emscripten::val::undefined();
        }
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
  emscripten::enum_<f3d::file_availability>("FileAvailability")
    .value("SUPPORTED", f3d::file_availability::SUPPORTED)
    .value("UNSUPPORTED_EXTENSION", f3d::file_availability::UNSUPPORTED_EXTENSION)
    .value("UNSUPPORTED_CONTENT", f3d::file_availability::UNSUPPORTED_CONTENT);

  emscripten::class_<f3d::scene>("Scene")
    .function(
      "supports", +[](f3d::scene& scene, const std::string& path) -> f3d::file_availability
      { return scene.supports(path); })
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
      "addMesh", +[](f3d::scene& scene, f3d::mesh_t& mesh) -> f3d::scene&
      { return scene.add(mesh); }, emscripten::return_value_policy::reference())
    .function(
      "addMeshView",
      +[](f3d::scene& scene, emscripten::val mesh) -> f3d::scene&
      {
        auto wrapped = std::make_shared<wasm_mesh_view>();
        if (mesh.hasOwnProperty("name"))
        {
          wrapped->Name = mesh["name"].as<std::string>();
        }
        if (mesh.hasOwnProperty("pointCount"))
        {
          wrapped->PointCount = mesh["pointCount"].as<size_t>();
        }
        if (mesh.hasOwnProperty("points"))
        {
          wrapped->Points = emscripten::vecFromJSArray<float>(mesh["points"]);
        }
        if (mesh.hasOwnProperty("normals"))
        {
          wrapped->Normals = emscripten::vecFromJSArray<float>(mesh["normals"]);
        }
        if (mesh.hasOwnProperty("textureCoordinates"))
        {
          wrapped->TextureCoordinates =
            emscripten::vecFromJSArray<float>(mesh["textureCoordinates"]);
        }
        if (mesh.hasOwnProperty("polygonOffsets"))
        {
          wrapped->PolygonOffsets =
            emscripten::vecFromJSArray<unsigned int>(mesh["polygonOffsets"]);
        }
        if (mesh.hasOwnProperty("polygonIndices"))
        {
          wrapped->PolygonIndices =
            emscripten::vecFromJSArray<unsigned int>(mesh["polygonIndices"]);
        }
        return scene.add(wrapped);
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
    .function("addLight", &f3d::scene::addLight)
    .function("getLightCount", &f3d::scene::getLightCount)
    .function("getLight", &f3d::scene::getLight)
    .function("updateLight", &f3d::scene::updateLight, emscripten::return_value_policy::reference())
    .function("removeLight", &f3d::scene::removeLight, emscripten::return_value_policy::reference())
    .function(
      "removeAllLights", &f3d::scene::removeAllLights, emscripten::return_value_policy::reference())
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
      +[](f3d::scene& scene) { return containerToJSArray(scene.getAnimationNames()); })
    .function(
      "getSceneHierarchy",
      +[](f3d::scene& scene) -> emscripten::val
      {
        emscripten::val jsArray = emscripten::val::array();
        for (const f3d::node_state_t& node : scene.getSceneHierarchy())
        {
          emscripten::val jsNode = emscripten::val::object();
          jsNode.set("id", node.id);
          jsNode.set("parentId", node.parentId);
          jsNode.set("level", node.level);
          jsNode.set("label", node.label);
          jsNode.set("visible", node.visible);
          jsNode.set("hasChildren", node.hasChildren);
          jsNode.set("collapsed", node.collapsed);
          jsArray.call<void>("push", jsNode);
        }
        return jsArray;
      })
    .function("setNodeVisibility", &f3d::scene::setNodeVisibility,
      emscripten::return_value_policy::reference())
    .function(
      "getSceneInfo",
      +[](f3d::scene& scene) -> emscripten::val
      {
        const f3d::scene_info_t info = scene.getSceneInfo();

        emscripten::val jsInfo = emscripten::val::object();
        jsInfo.set("numberOfFiles", info.numberOfFiles);
        jsInfo.set("numberOfActors", info.numberOfActors);
        jsInfo.set("numberOfPoints", info.numberOfPoints);
        jsInfo.set("numberOfCells", info.numberOfCells);
        return jsInfo;
      });

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
  emscripten::class_<f3d::camera_state_t>("CameraState")
    .constructor<>()
    .property("position", &f3d::camera_state_t::position)
    .property("focalPoint", &f3d::camera_state_t::focalPoint)
    .property("viewUp", &f3d::camera_state_t::viewUp)
    .property("viewAngle", &f3d::camera_state_t::viewAngle);

  emscripten::class_<f3d::camera>("Camera")
    .property(
      "position", +[](const f3d::camera& cam) -> f3d::point3_t { return cam.getPosition(); },
      +[](f3d::camera& cam, const f3d::point3_t& position) { cam.setPosition(position); })
    .property(
      "focalPoint", +[](const f3d::camera& cam) -> f3d::point3_t { return cam.getFocalPoint(); },
      +[](f3d::camera& cam, const f3d::point3_t& focalPoint) { cam.setFocalPoint(focalPoint); })
    .property(
      "viewUp", +[](const f3d::camera& cam) -> f3d::vector3_t { return cam.getViewUp(); },
      +[](f3d::camera& cam, f3d::vector3_t viewUp) { cam.setViewUp(viewUp); })
    .property("viewAngle",
      static_cast<f3d::angle_deg_t (f3d::camera::*)() const>(&f3d::camera::getViewAngle),
      &f3d::camera::setViewAngle)
    .property(
      "state", +[](const f3d::camera& cam) -> f3d::camera_state_t { return cam.getState(); },
      +[](f3d::camera& cam, const f3d::camera_state_t& state) -> f3d::camera&
      { return cam.setState(state); })
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
  // getType, isOffscreen, setPosition, getPosition, getLeft, getTop, setIcon, setWindowName
  emscripten::class_<f3d::window>("Window")
    .function("getCamera", &f3d::window::getCamera, emscripten::return_value_policy::reference())
    .function("render", &f3d::window::render)
    .function("renderToImage", &f3d::window::renderToImage)
    .function("setSize", &f3d::window::setSize, emscripten::return_value_policy::reference())
    .property(
      "size",
      +[](const f3d::window& win) -> emscripten::val { return pairToJSArray(win.getSize()); },
      +[](f3d::window& win, emscripten::val jsArray)
      { win.setSize(jsArray[0].as<int>(), jsArray[1].as<int>()); })
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
      })
    .function("getDPIScale", &f3d::window::getDPIScale);

  // f3d::interaction_bind_t
  emscripten::enum_<f3d::interaction_bind_t::ModifierKeys>("InteractionBindModifierKeys")
    .value("ANY", f3d::interaction_bind_t::ModifierKeys::ANY)
    .value("NONE", f3d::interaction_bind_t::ModifierKeys::NONE)
    .value("CTRL", f3d::interaction_bind_t::ModifierKeys::CTRL)
    .value("SHIFT", f3d::interaction_bind_t::ModifierKeys::SHIFT)
    .value("CTRL_SHIFT", f3d::interaction_bind_t::ModifierKeys::CTRL_SHIFT);

  emscripten::class_<f3d::interaction_bind_t>("InteractionBind")
    .constructor<>()
    .property("mod", &f3d::interaction_bind_t::mod)
    .property("inter", &f3d::interaction_bind_t::inter);

  // f3d::interactor
  emscripten::enum_<f3d::interactor::AnimationDirection>("InteractorAnimationDirection")
    .value("FORWARD", f3d::interactor::AnimationDirection::FORWARD)
    .value("BACKWARD", f3d::interactor::AnimationDirection::BACKWARD);

  emscripten::enum_<f3d::interactor::BindingType>("InteractorBindingType")
    .value("CYCLIC", f3d::interactor::BindingType::CYCLIC)
    .value("NUMERICAL", f3d::interactor::BindingType::NUMERICAL)
    .value("TOGGLE", f3d::interactor::BindingType::TOGGLE)
    .value("OTHER", f3d::interactor::BindingType::OTHER);

  emscripten::enum_<f3d::interactor::MouseButton>("InteractorMouseButton")
    .value("LEFT", f3d::interactor::MouseButton::LEFT)
    .value("RIGHT", f3d::interactor::MouseButton::RIGHT)
    .value("MIDDLE", f3d::interactor::MouseButton::MIDDLE);

  emscripten::enum_<f3d::interactor::WheelDirection>("InteractorWheelDirection")
    .value("FORWARD", f3d::interactor::WheelDirection::FORWARD)
    .value("BACKWARD", f3d::interactor::WheelDirection::BACKWARD)
    .value("LEFT", f3d::interactor::WheelDirection::LEFT)
    .value("RIGHT", f3d::interactor::WheelDirection::RIGHT);

  emscripten::enum_<f3d::interactor::InputAction>("InteractorInputAction")
    .value("PRESS", f3d::interactor::InputAction::PRESS)
    .value("RELEASE", f3d::interactor::InputAction::RELEASE);

  emscripten::enum_<f3d::interactor::InputModifier>("InteractorInputModifier")
    .value("NONE", f3d::interactor::InputModifier::NONE)
    .value("CTRL", f3d::interactor::InputModifier::CTRL)
    .value("SHIFT", f3d::interactor::InputModifier::SHIFT)
    .value("CTRL_SHIFT", f3d::interactor::InputModifier::CTRL_SHIFT);

  emscripten::class_<f3d::interactor_state_t>("InteractorState")
    .property("animationTime", &f3d::interactor_state_t::animationTime);

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
      "initBindings", &f3d::interactor::initBindings, emscripten::return_value_policy::reference())
    .function(
      "addBinding",
      +[](f3d::interactor& interactor, const f3d::interaction_bind_t& bind,
         const emscripten::val& commands) -> f3d::interactor&
      {
        const std::vector<std::string> commandList =
          emscripten::vecFromJSArray<std::string>(commands);
        return interactor.addBinding(bind, commandList);
      },
      emscripten::return_value_policy::reference())
    .function(
      "addBinding",
      +[](f3d::interactor& interactor, const f3d::interaction_bind_t& bind,
         const emscripten::val& commands, std::string group, const emscripten::val& callback,
         f3d::interactor::BindingType type, bool notify) -> f3d::interactor&
      {
        auto wrapCallback = [=]() -> std::pair<std::string, std::string>
        {
          emscripten::val result = callback();
          if (!result.isArray() || result["length"].as<unsigned int>() != 2)
          {
            throw std::runtime_error("Callback must return an array of two strings");
          }
          return { result[0].as<std::string>(), result[1].as<std::string>() };
        };
        const std::vector<std::string> commandList =
          emscripten::vecFromJSArray<std::string>(commands);
        return interactor.addBinding(bind, commandList, group, wrapCallback, type, notify);
      },
      emscripten::return_value_policy::reference())
    .function("removeBinding", &f3d::interactor::removeBinding,
      emscripten::return_value_policy::reference())
    .function(
      "getBindGroups", +[](const f3d::interactor& interactor) -> emscripten::val
      { return containerToJSArray(interactor.getBindGroups()); })
    .function(
      "getBinds", +[](const f3d::interactor& interactor) -> emscripten::val
      { return containerToJSArray(interactor.getBinds()); })
    .function(
      "getBindingDocumentation",
      +[](const f3d::interactor& interactor, const f3d::interaction_bind_t& bind) -> emscripten::val
      {
        auto bindingDoc = interactor.getBindingDocumentation(bind);
        std::vector<std::string> docStrings;
        docStrings.emplace_back(bindingDoc.first);
        docStrings.emplace_back(bindingDoc.second);
        return containerToJSArray(docStrings);
      })
    .function("getBindingType", &f3d::interactor::getBindingType)
    .function(
      "toggleAnimation", +[](f3d::interactor& interactor) -> f3d::interactor&
      { return interactor.toggleAnimation(); }, emscripten::return_value_policy::reference())
    .function(
      "toggleAnimation",
      +[](f3d::interactor& interactor, emscripten::val direction) -> f3d::interactor&
      { return interactor.toggleAnimation(direction.as<f3d::interactor::AnimationDirection>()); },
      emscripten::return_value_policy::reference())
    .function(
      "startAnimation", +[](f3d::interactor& interactor) -> f3d::interactor&
      { return interactor.startAnimation(); }, emscripten::return_value_policy::reference())
    .function(
      "startAnimation",
      +[](f3d::interactor& interactor, emscripten::val direction) -> f3d::interactor&
      { return interactor.startAnimation(direction.as<f3d::interactor::AnimationDirection>()); },
      emscripten::return_value_policy::reference())
    .function("stopAnimation", &f3d::interactor::stopAnimation,
      emscripten::return_value_policy::reference())
    .function("isPlayingAnimation", &f3d::interactor::isPlayingAnimation)
    .function("getAnimationDirection", &f3d::interactor::getAnimationDirection)
    .function("enableCameraMovement", &f3d::interactor::enableCameraMovement,
      emscripten::return_value_policy::reference())
    .function("disableCameraMovement", &f3d::interactor::disableCameraMovement,
      emscripten::return_value_policy::reference())

    .function("triggerModUpdate", &f3d::interactor::triggerModUpdate,
      emscripten::return_value_policy::reference())
    .function("triggerMouseButton", &f3d::interactor::triggerMouseButton,
      emscripten::return_value_policy::reference())
    .function("triggerMousePosition", &f3d::interactor::triggerMousePosition,
      emscripten::return_value_policy::reference())
    .function("triggerMouseWheel", &f3d::interactor::triggerMouseWheel,
      emscripten::return_value_policy::reference())
    .function(
      "triggerKeyboardKey",
      +[](f3d::interactor& interactor, f3d::interactor::InputAction action,
         std::string keySym) -> f3d::interactor&
      { return interactor.triggerKeyboardKey(action, keySym); },
      emscripten::return_value_policy::reference())
    .function("triggerTextCharacter", &f3d::interactor::triggerTextCharacter,
      emscripten::return_value_policy::reference())
    .function("triggerEventLoop", &f3d::interactor::triggerEventLoop,
      emscripten::return_value_policy::reference())
    .function("triggerNotification", &f3d::interactor::triggerNotification,
      emscripten::return_value_policy::reference())
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
      })
    .function(
      "playInteraction",
      +[](f3d::interactor& interactor, const std::string& path, double deltaTime) -> bool
      { return interactor.playInteraction(path, deltaTime); })
    .function(
      "recordInteraction", +[](f3d::interactor& interactor, const std::string& path) -> bool
      { return interactor.recordInteraction(path); })
    .function(
      "setEventLoopUserCallback",
      +[](f3d::interactor& interactor, const emscripten::val& callback) -> f3d::interactor&
      {
        if (callback.isUndefined() || callback.isNull())
        {
          return interactor.setEventLoopUserCallback(nullptr);
        }

        return interactor.setEventLoopUserCallback(
          [=](f3d::interactor_state_t state) { callback(state); });
      },
      emscripten::return_value_policy::reference())
    .function(
      "start", +[](f3d::interactor& interactor) -> f3d::interactor& { return interactor.start(); },
      emscripten::return_value_policy::reference())
    .function("stop", &f3d::interactor::stop, emscripten::return_value_policy::reference())
    .function("requestRender", &f3d::interactor::requestRender,
      emscripten::return_value_policy::reference())
    .function(
      "requestStop", &f3d::interactor::requestStop, emscripten::return_value_policy::reference());

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
    .property(
      "cachePath",
      +[](const f3d::engine& engine) -> std::string { return engine.getCachePath().string(); },
      +[](f3d::engine& engine, const std::string& path) { engine.setCachePath(path); })
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
