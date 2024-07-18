#include "options.h"
#include "options_struct.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include "vtkF3DConfigure.h"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <iostream>

namespace f3d
{
//----------------------------------------------------------------------------
class options::internals
{
public:
/*  using OptionVariant =
    std::variant<bool, int, double, std::string, std::vector<int>, std::vector<double>>;

  template<typename T, typename U>
  struct IsTypeValid;

  template<typename T, typename... Ts>
  struct IsTypeValid<T, std::variant<Ts...>> : public std::disjunction<std::is_same<T, Ts>...>
  {
  };

  template<typename T>
  void init(const std::string& name, const T& value)
  {
    static_assert(IsTypeValid<T, OptionVariant>::value);
    this->Options[name] = value;
  }

  template<typename T>
  void set(const std::string& name, const T& value)
  {
    static_assert(!std::is_array_v<T> && !std::is_pointer_v<T>);
    try
    {
      T& opt = std::get<T>(this->Options.at(name));
      opt = value;
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to set option ", name, " with incompatible type");
    }
    catch (const std::out_of_range&)
    {
      log::error("Option ", name, " does not exist");
    }
  }

  template<typename T>
  void get(const std::string& name, T& value) const
  {
    try
    {
      value = std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to get option ", name, " with incompatible type");
      return;
    }
    catch (const std::out_of_range&)
    {
      log::error("Option ", name, " does not exist");
      return;
    }
  }

  template<typename T>
  T get(const std::string& name) const
  {
    T val = {};
    this->get(name, val);
    return val;
  }

  template<typename T>
  T& getRef(const std::string& name)
  {
    try
    {
      return std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      throw options::incompatible_exception(
        "Trying to get option reference " + name + " with incompatible type");
    }
    catch (const std::out_of_range&)
    {
      throw options::inexistent_exception("Option " + name + " does not exist");
    }
  }

  void initNew(const std::string& name, option_variant_t value)
  {
    // TODO do we actually need this call ?
    this->setVariant(name, value);
  }*/

  void setVariant(const std::string& name, option_variant_t value)
  {
    // TODO GENERATE
    if (name == "scene.up_direction")
    {
      this->OptionStruct.scene.up_direction = std::get<std::string>(value);
    }
    else if (name == "scene.animation.autoplay")
    {
      this->OptionStruct.scene.animation.autoplay = std::get<bool>(value);
    }
    else if (name == "scene.animation.index")
    {
      this->OptionStruct.scene.animation.index = std::get<int>(value);
    }
    else if (name == "scene.animation.speed_factor")
    {
      this->OptionStruct.scene.animation.speed_factor = std::get<double>(value);
    }
    else if (name == "scene.animation.time")
    {
      this->OptionStruct.scene.animation.time = std::get<double>(value);
    }
    else if (name == "scene.animation.frame_rate")
    {
      this->OptionStruct.scene.animation.frame_rate = std::get<double>(value);
    }
    else if (name == "scene.camera.index")
    {
      this->OptionStruct.scene.camera.index = std::get<int>(value);
    }
    else if (name == "scene.camera.orthographic")
    {
      this->OptionStruct.scene.camera.orthographic = std::get<bool>(value);
    }

    else if (name == "render.show_edges")
    {
      this->OptionStruct.render.show_edges = std::get<bool>(value);
    }
    else if (name == "render.line_width")
    {
      this->OptionStruct.render.line_width = std::get<double>(value);
    }
    else if (name == "render.point_size")
    {
      this->OptionStruct.render.point_size = std::get<double>(value);
    }
    else if (name == "render.backface_type")
    {
      this->OptionStruct.render.backface_type = std::get<std::string>(value);
    }
    else if (name == "render.grid.enable")
    {
      this->OptionStruct.render.grid.enable = std::get<bool>(value);
    }
    else if (name == "render.grid.absolute")
    {
      this->OptionStruct.render.grid.absolute = std::get<bool>(value);
    }
    else if (name == "render.grid.unit")
    {
      this->OptionStruct.render.grid.unit = std::get<double>(value);
    }
    else if (name == "render.grid.subdivisions")
    {
      this->OptionStruct.render.grid.subdivisions = std::get<int>(value);
    }
    else if (name == "render.grid.color")
    {
      this->OptionStruct.render.grid.color = std::get<std::vector<double>>(value);
    }
    else if (name == "render.raytracing.enable")
    {
      this->OptionStruct.render.raytracing.enable = std::get<bool>(value);
    }
    else if (name == "render.raytracing.denoise")
    {
      this->OptionStruct.render.raytracing.denoise = std::get<bool>(value);
    }
    else if (name == "render.raytracing.samples")
    {
      this->OptionStruct.render.raytracing.samples = std::get<int>(value);
    }
    else if (name == "render.effect.translucency_support")
    {
      this->OptionStruct.render.effect.translucency_support = std::get<bool>(value);
    }
    else if (name == "render.effect.anti_aliasing")
    {
      this->OptionStruct.render.effect.anti_aliasing = std::get<bool>(value);
    }
    else if (name == "render.effect.ambient_occlusion")
    {
      this->OptionStruct.render.effect.ambient_occlusion = std::get<bool>(value);
    }
    else if (name == "render.effect.tone_mapping")
    {
      this->OptionStruct.render.effect.tone_mapping = std::get<bool>(value);
    }
    else if (name == "render.effect.final_shader")
    {
      this->OptionStruct.render.effect.final_shader = std::get<std::string>(value);
    }
    else if (name == "render.hdri.file")
    {
      this->OptionStruct.render.hdri.file = std::get<std::string>(value);
    }
    else if (name == "render.hdri.ambient")
    {
      this->OptionStruct.render.hdri.ambient = std::get<bool>(value);
    }
    else if (name == "render.background.color")
    {
      this->OptionStruct.render.background.color = std::get<std::vector<double>>(value);
    }
    else if (name == "render.background.skybox")
    {
      this->OptionStruct.render.background.skybox = std::get<bool>(value);
    }
    else if (name == "render.background.blur")
    {
      this->OptionStruct.render.background.blur = std::get<bool>(value);
    }
    else if (name == "render.background.blur_coc")
    {
      this->OptionStruct.render.background.blur_coc = std::get<double>(value);
    }
    else if (name == "render.light.intensity")
    {
      this->OptionStruct.render.light.intensity = std::get<double>(value);
    }

    else if (name == "ui.scalar_bar")
    {
      this->OptionStruct.ui.scalar_bar = std::get<bool>(value);
    }
    else if (name == "ui.filename")
    {
      this->OptionStruct.ui.filename = std::get<bool>(value);
    }
    else if (name == "ui.filename_info")
    {
      this->OptionStruct.ui.filename_info = std::get<std::string>(value);
    }
    else if (name == "ui.fps")
    {
      this->OptionStruct.ui.fps = std::get<bool>(value);
    }
    else if (name == "ui.cheatsheet")
    {
      this->OptionStruct.ui.cheatsheet = std::get<bool>(value);
    }
    else if (name == "ui.dropzone")
    {
      this->OptionStruct.ui.dropzone = std::get<bool>(value);
    }
    else if (name == "ui.dropzone_info")
    {
      this->OptionStruct.ui.dropzone_info = std::get<std::string>(value);
    }
    else if (name == "ui.metadata")
    {
      this->OptionStruct.ui.metadata = std::get<bool>(value);
    }
    else if (name == "ui.font_file")
    {
      this->OptionStruct.ui.font_file = std::get<std::string>(value);
    }
    else if (name == "ui.loader_progress")
    {
      this->OptionStruct.ui.loader_progress = std::get<bool>(value);
    }
    else if (name == "ui.animation_progress")
    {
      this->OptionStruct.ui.animation_progress = std::get<bool>(value);
    }

    else if (name == "model.matcap.texture")
    {
      this->OptionStruct.model.matcap.texture = std::get<std::string>(value);
    }
    else if (name == "model.color.opacity")
    {
      this->OptionStruct.model.color.opacity = std::get<double>(value);
    }
    else if (name == "model.color.rgb")
    {
      this->OptionStruct.model.color.rgb = std::get<std::vector<double>>(value);
    }
    else if (name == "model.color.texture")
    {
      this->OptionStruct.model.color.texture = std::get<std::string>(value);
    }
    else if (name == "model.emissive.factor")
    {
      this->OptionStruct.model.emissive.factor = std::get<std::vector<double>>(value);
    }
    else if (name == "model.emissive.texture")
    {
      this->OptionStruct.model.emissive.texture = std::get<std::string>(value);
    }
    else if (name == "model.normal.scale")
    {
      this->OptionStruct.model.normal.scale = std::get<double>(value);
    }
    else if (name == "model.normal.texture")
    {
      this->OptionStruct.model.normal.texture = std::get<std::string>(value);
    }
    else if (name == "model.material.metallic")
    {
      this->OptionStruct.model.material.metallic = std::get<double>(value);
    }
    else if (name == "model.material.roughness")
    {
      this->OptionStruct.model.material.roughness = std::get<double>(value);
    }
    else if (name == "model.material.texture")
    {
      this->OptionStruct.model.material.texture = std::get<std::string>(value);
    }
    else if (name == "model.scivis.cells")
    {
      this->OptionStruct.model.scivis.cells = std::get<bool>(value);
    }
    else if (name == "model.scivis.array_name")
    {
      this->OptionStruct.model.scivis.array_name = std::get<std::string>(value);
    }
    else if (name == "model.scivis.component")
    {
      this->OptionStruct.model.scivis.component = std::get<int>(value);
    }
    else if (name == "model.scivis.colormap")
    {
      this->OptionStruct.model.scivis.colormap = std::get<std::vector<double>>(value);
    }
    else if (name == "model.scivis.range")
    {
      this->OptionStruct.model.scivis.range = std::get<std::vector<double>>(value);
    }
    else if (name == "model.point_sprites.enable")
    {
      this->OptionStruct.model.point_sprites.enable = std::get<bool>(value);
    }
    else if (name == "model.point_sprites.sprites_type")
    {
      this->OptionStruct.model.point_sprites.sprites_type = std::get<std::string>(value);
    }
    else if (name == "model.volume.enable")
    {
      this->OptionStruct.model.volume.enable = std::get<bool>(value);
    }
    else if (name == "model.volume.inverse")
    {
      this->OptionStruct.model.volume.inverse = std::get<bool>(value);
    }

    else if (name == "interactor.axis")
    {
      this->OptionStruct.interactor.axis = std::get<bool>(value);
    }
    else if (name == "interactor.trackball")
    {
      this->OptionStruct.interactor.trackball = std::get<bool>(value);
    }
    else if (name == "interactor.invert_zoom")
    {
      this->OptionStruct.interactor.invert_zoom = std::get<bool>(value);
    }
    else
    {
      //TODO ERROR MGT
      std::cout<<"ERROR WITH SET VARIANT: "<<name<<std::endl;
    }
  }

  option_variant_t getVariant(const std::string& name)
  {
    // TODO GENERATE
    option_variant_t var;
    if (name == "scene.up_direction")
    {
      var = this->OptionStruct.scene.up_direction;
    }
    else if (name == "scene.animation.autoplay")
    {
      var = this->OptionStruct.scene.animation.autoplay;
    }
    else if (name == "scene.animation.index")
    {
      var = this->OptionStruct.scene.animation.index;
    }
    else if (name == "scene.animation.speed_factor")
    {
      var = this->OptionStruct.scene.animation.speed_factor;
    }
    else if (name == "scene.animation.time")
    {
      var = this->OptionStruct.scene.animation.time;
    }
    else if (name == "scene.animation.frame_rate")
    {
      var = this->OptionStruct.scene.animation.frame_rate;
    }
    else if (name == "scene.camera.index")
    {
      var = this->OptionStruct.scene.camera.index;
    }
    else if (name == "scene.camera.orthographic")
    {
      var = this->OptionStruct.scene.camera.orthographic;
    }

    else if (name == "render.show_edges")
    {
      var = this->OptionStruct.render.show_edges;
    }
    else if (name == "render.line_width")
    {
      var = this->OptionStruct.render.line_width;
    }
    else if (name == "render.point_size")
    {
      var = this->OptionStruct.render.point_size;
    }
    else if (name == "render.backface_type")
    {
      var = this->OptionStruct.render.backface_type;
    }
    else if (name == "render.grid.enable")
    {
      var = this->OptionStruct.render.grid.enable;
    }
    else if (name == "render.grid.absolute")
    {
      var = this->OptionStruct.render.grid.absolute;
    }
    else if (name == "render.grid.unit")
    {
      var = this->OptionStruct.render.grid.unit;
    }
    else if (name == "render.grid.subdivisions")
    {
      var = this->OptionStruct.render.grid.subdivisions;
    }
    else if (name == "render.grid.color")
    {
      var = this->OptionStruct.render.grid.color;
    }
    else if (name == "render.raytracing.enable")
    {
      var = this->OptionStruct.render.raytracing.enable;
    }
    else if (name == "render.raytracing.denoise")
    {
      var = this->OptionStruct.render.raytracing.denoise;
    }
    else if (name == "render.raytracing.samples")
    {
      var = this->OptionStruct.render.raytracing.samples;
    }
    else if (name == "render.effect.translucency_support")
    {
      var = this->OptionStruct.render.effect.translucency_support;
    }
    else if (name == "render.effect.anti_aliasing")
    {
      var = this->OptionStruct.render.effect.anti_aliasing;
    }
    else if (name == "render.effect.ambient_occlusion")
    {
      var = this->OptionStruct.render.effect.ambient_occlusion;
    }
    else if (name == "render.effect.tone_mapping")
    {
      var = this->OptionStruct.render.effect.tone_mapping;
    }
    else if (name == "render.effect.final_shader")
    {
      var = this->OptionStruct.render.effect.final_shader;
    }
    else if (name == "render.hdri.file")
    {
      var = this->OptionStruct.render.hdri.file;
    }
    else if (name == "render.hdri.ambient")
    {
      var = this->OptionStruct.render.hdri.ambient;
    }
    else if (name == "render.background.color")
    {
      var = this->OptionStruct.render.background.color;
    }
    else if (name == "render.background.skybox")
    {
      var = this->OptionStruct.render.background.skybox;
    }
    else if (name == "render.background.blur")
    {
      var = this->OptionStruct.render.background.blur;
    }
    else if (name == "render.background.blur_coc")
    {
      var = this->OptionStruct.render.background.blur_coc;
    }
    else if (name == "render.light.intensity")
    {
      var = this->OptionStruct.render.light.intensity;
    }

    else if (name == "ui.scalar_bar")
    {
      var = this->OptionStruct.ui.scalar_bar;
    }
    else if (name == "ui.filename")
    {
      var = this->OptionStruct.ui.filename;
    }
    else if (name == "ui.filename_info")
    {
      var = this->OptionStruct.ui.filename_info;
    }
    else if (name == "ui.fps")
    {
      var = this->OptionStruct.ui.fps;
    }
    else if (name == "ui.cheatsheet")
    {
      var = this->OptionStruct.ui.cheatsheet;
    }
    else if (name == "ui.dropzone")
    {
      var = this->OptionStruct.ui.dropzone;
    }
    else if (name == "ui.dropzone_info")
    {
      var = this->OptionStruct.ui.dropzone_info;
    }
    else if (name == "ui.metadata")
    {
      var = this->OptionStruct.ui.metadata;
    }
    else if (name == "ui.font_file")
    {
      var = this->OptionStruct.ui.font_file;
    }
    else if (name == "ui.loader_progress")
    {
      var = this->OptionStruct.ui.loader_progress;
    }
    else if (name == "ui.animation_progress")
    {
      var = this->OptionStruct.ui.animation_progress;
    }

    else if (name == "model.matcap.texture")
    {
      var = this->OptionStruct.model.matcap.texture;
    }
    else if (name == "model.color.opacity")
    {
      var = this->OptionStruct.model.color.opacity;
    }
    else if (name == "model.color.rgb")
    {
      var = this->OptionStruct.model.color.rgb;
    }
    else if (name == "model.color.texture")
    {
      var = this->OptionStruct.model.color.texture;
    }
    else if (name == "model.emissive.factor")
    {
      var = this->OptionStruct.model.emissive.factor;
    }
    else if (name == "model.emissive.texture")
    {
      var = this->OptionStruct.model.emissive.texture;
    }
    else if (name == "model.normal.scale")
    {
      var = this->OptionStruct.model.normal.scale;
    }
    else if (name == "model.normal.texture")
    {
      var = this->OptionStruct.model.normal.texture;
    }
    else if (name == "model.material.metallic")
    {
      var = this->OptionStruct.model.material.metallic;
    }
    else if (name == "model.material.roughness")
    {
      var = this->OptionStruct.model.material.roughness;
    }
    else if (name == "model.material.texture")
    {
      var = this->OptionStruct.model.material.texture;
    }
    else if (name == "model.scivis.cells")
    {
      var = this->OptionStruct.model.scivis.cells;
    }
    else if (name == "model.scivis.array_name")
    {
      var = this->OptionStruct.model.scivis.array_name;
    }
    else if (name == "model.scivis.component")
    {
      var = this->OptionStruct.model.scivis.component;
    }
    else if (name == "model.scivis.colormap")
    {
      var = this->OptionStruct.model.scivis.colormap;
    }
    else if (name == "model.scivis.range")
    {
      var = this->OptionStruct.model.scivis.range;
    }
    else if (name == "model.point_sprites.enable")
    {
      var = this->OptionStruct.model.point_sprites.enable;
    }
    else if (name == "model.point_sprites.sprites_type")
    {
      var = this->OptionStruct.model.point_sprites.sprites_type;
    }
    else if (name == "model.volume.enable")
    {
      var = this->OptionStruct.model.volume.enable;
    }
    else if (name == "model.volume.inverse")
    {
      var = this->OptionStruct.model.volume.inverse;
    }

    else if (name == "interactor.axis")
    {
      var = this->OptionStruct.interactor.axis;
    }
    else if (name == "interactor.trackball")
    {
      var = this->OptionStruct.interactor.trackball;
    }
    else if (name == "interactor.invert_zoom")
    {
      var = this->OptionStruct.interactor.invert_zoom;
    }
    else
    {
      //TODO error mgt
      std::cout<<"ERROR WITH GET VARIANT: "<<name<<std::endl;
    }
    return var;
  }

  std::vector<std::string> getNames()
  {
    std::vector<std::string> vec{
    "scene.up_direction",

    "scene.animation.autoplay",

     "scene.animation.index",

     "scene.animation.speed_factor",

     "scene.animation.time",

     "scene.animation.frame_rate",

     "scene.camera.index",

     "scene.camera.orthographic",

     "render.show_edges",

     "render.line_width",

     "render.point_size",

     "render.backface_type",

     "render.grid.enable",

     "render.grid.absolute",

     "render.grid.unit",

     "render.grid.subdivisions",

     "render.grid.color",

     "render.raytracing.enable",

     "render.raytracing.denoise",

     "render.raytracing.samples",

     "render.effect.translucency_support",

     "render.effect.anti_aliasing",

     "render.effect.ambient_occlusion",

     "render.effect.tone_mapping",

     "render.effect.final_shader",

     "render.hdri.file",

     "render.hdri.ambient",

     "render.background.color",

     "render.background.skybox",

     "render.background.blur",

     "render.background.blur_coc",

     "render.light.intensity",

     "ui.scalar_bar",

     "ui.filename",

     "ui.filename_info",

     "ui.fps",

     "ui.cheatsheet",

     "ui.dropzone",

     "ui.dropzone_info",

     "ui.metadata",

     "ui.font_file",

     "ui.loader_progress",

     "ui.animation_progress",

     "model.matcap.texture",

     "model.color.opacity",
     
     "model.color.rgb",

     "model.color.texture",

     "model.emissive.factor",

     "model.emissive.texture",

     "model.normal.scale",

     "model.normal.texture",

     "model.material.metallic",

     "model.material.roughness",

     "model.material.texture",

     "model.scivis.cells",

     "model.scivis.array_name",

     "model.scivis.component",

     "model.scivis.colormap",

     "model.scivis.range",

     "model.point_sprites.enable",

     "model.point_sprites.sprites_type",

     "model.volume.enable",

     "model.volume.inverse",

     "interactor.axis",

     "interactor.trackball",

     "interactor.invert_zoom",

    };
    return vec;
  }

  void setString(const std::string& name, std::string value)
  {
    option_variant_t var = this->getVariant(name);
    if (std::holds_alternative<bool>(var))
    {
      // TODO implement proper parsing
      bool b1;
      bool b2;
      std::istringstream(value) >> b1;
      std::istringstream(value) >> std::boolalpha >> b2;
      var = b1 || b2;
    }
    if (std::holds_alternative<int>(var))
    {
      // TODO implement proper parsing
      var = std::stoi(value);
    }
    else if (std::holds_alternative<double>(var))
    {
      // TODO implement proper parsing
      var = std::stod(value);
    }
    else if (std::holds_alternative<std::string>(var))
    {
      var = value;
    }
    else if (std::holds_alternative<std::vector<double>>(var))
    {
      // TODO implement proper parsing
      std::istringstream split(value);
      std::vector<double>& vec = std::get<std::vector<double>>(var);
      vec.clear();
      for (std::string each; std::getline(split, each, ','); vec.push_back(std::stod(each)));
    }
    else
    {
      // TODO implement error mgt
    }
    this->setVariant(name, var);
  }

  std::string getString(const std::string& name)
  {
    option_variant_t var = this->getVariant(name);
    std::string str;
    try
    {
      if (std::holds_alternative<bool>(var))
      {
        str = std::to_string(std::get<bool>(var));
      }
      if (std::holds_alternative<int>(var))
      {
        str = std::to_string(std::get<int>(var));
      }
      else if (std::holds_alternative<double>(var))
      {
        str = std::to_string(std::get<double>(var));
      }
      else if (std::holds_alternative<std::string>(var))
      {
        str = std::get<std::string>(var);
      }
      else if (std::holds_alternative<std::vector<double>>(var))
      {
        std::vector<double> vec = std::get<std::vector<double>>(var);
        std::ostringstream stream;
        unsigned int i = 0;
        for (auto& elem : vec)
        {
          stream << ((i > 0) ? "," : "") << std::to_string(elem);
          i++;
        }
        stream << '\n';
        str = stream.str();
      }
      else
      {
        // TODO implement error mgt
      }
    }
    catch (const std::bad_variant_access&)
    {
      throw options::incompatible_exception(
        "Trying to get option reference " + name + " with incompatible type");
    }
    return str;
  }

//  std::map<std::string, OptionVariant> Options;

  options_struct OptionStruct;
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::internals)
{
  detail::init::initialize();
/*
  // Scene
  this->Internals->initNew("scene.animation.autoplay", false); // bool
  this->Internals->initNew("scene.animation.index", 0); // int
  this->Internals->init("scene.animation.speed-factor", 1.0); // ratio
  this->Internals->init("scene.animation.time", 0.0); // double
  this->Internals->init("scene.animation.frame-rate", 60.0); // double
  this->Internals->init("scene.camera.index", -1); // int
  this->Internals->init("scene.up-direction", std::string("+Y")); // direction

  // Render
  this->Internals->init("render.show-edges", false); // bool
//  this->Internals->init("render.line-width", 1.0);
  this->Internals->initNew("render.line_width", 1.0); // double
  this->Internals->init("render.point-size", 10.0); // double
  this->Internals->init("render.grid.enable", false); // bool
  this->Internals->init("render.grid.absolute", false); // bool
  this->Internals->init("render.grid.unit", 0.0); // double
  this->Internals->init("render.grid.subdivisions", 10); // int
  this->Internals->init("render.grid.color", std::vector<double>{ 0.0, 0.0, 0.0 }); // color
//  this->Internals->init("render.backface-type", std::string("default")); // string
  this->Internals->initNew("render.backface_type", std::string("default")); // string

  this->Internals->init("render.raytracing.enable", false); // bool
  this->Internals->init("render.raytracing.denoise", false); // bool
  this->Internals->init("render.raytracing.samples", 5); // int

  this->Internals->init("render.effect.translucency-support", false); // bool
  this->Internals->init("render.effect.anti-aliasing", false); // bool
  this->Internals->init("render.effect.ambient-occlusion", false); // bool
  this->Internals->init("render.effect.tone-mapping", false); // bool
  this->Internals->init("render.effect.final-shader", std::string()); // string

  this->Internals->init("render.hdri.file", std::string()); // path
  this->Internals->init("render.hdri.ambient", false); // bool
  this->Internals->init("render.background.color", std::vector<double>{ 0.2, 0.2, 0.2 }); // color
  this->Internals->init("render.background.skybox", false); // bool
  this->Internals->init("render.background.blur", false); // bool
  this->Internals->init("render.background.blur.coc", 20.0); // double
  this->Internals->init("render.light.intensity", 1.); // double

  // UI
  this->Internals->init("ui.bar", false); // bool
  this->Internals->init("ui.filename", false); // bool
  this->Internals->init("ui.filename-info", std::string()); // string
//  this->Internals->init("ui.fps", false); // bool
  this->Internals->initNew("ui.fps", false); // bool
  this->Internals->init("ui.cheatsheet", false); // bool
  this->Internals->init("ui.dropzone", false); // bool
  this->Internals->init("ui.dropzone-info", std::string()); // string
  this->Internals->init("ui.metadata", false); // bool
  this->Internals->init("ui.font-file", std::string()); // path
  this->Internals->init("ui.loader-progress", false); // bool
  this->Internals->init("ui.animation-progress", false); // bool

  // Model
  this->Internals->init("model.matcap.texture", std::string()); // path

  this->Internals->init("model.color.opacity", 1.0); // ratio
  // XXX: Not compatible with scivis: https://github.com/f3d-app/f3d/issues/347
  this->Internals->init("model.color.rgb", std::vector<double>{ 1., 1., 1. }); // color
  // XXX: Artifacts when using with scivis: https://github.com/f3d-app/f3d/issues/348
  this->Internals->init("model.color.texture", std::string()); // path

  this->Internals->init("model.emissive.factor", std::vector<double>{ 1., 1., 1. }); // color
  this->Internals->init("model.emissive.texture", std::string()); // path

  this->Internals->init("model.normal.texture", std::string()); // path
  this->Internals->init("model.normal.scale", 1.0); // ratio ?

  this->Internals->init("model.material.metallic", 0.0); // ratio ?
  this->Internals->init("model.material.roughness", 0.3); // ratio ?
  this->Internals->init("model.material.texture", std::string()); // path

  this->Internals->init("model.scivis.cells", false); // bool
  this->Internals->init("model.scivis.array-name", F3D_RESERVED_STRING); // string
  this->Internals->init("model.scivis.component", -1); // int
  this->Internals->init("model.scivis.colormap",
    std::vector<double>{
      0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 }); // rgba_colormap
//  this->Internals->init("model.scivis.range", std::vector<double>{ 0 });
  this->Internals->initNew("model.scivis.range", std::vector<double>{ 0 }); // vector_double

  // XXX: Rename into a "rendering-mode" option: https://github.com/f3d-app/f3d/issues/345
  this->Internals->init("model.point-sprites.enable", false); // bool
  this->Internals->init("model.point-sprites.type", std::string("sphere")); // string
  this->Internals->init("model.volume.enable", false); // bool
  this->Internals->init("model.volume.inverse", false); // bool

  // Camera projection
  this->Internals->init("scene.camera.orthographic", false); // bool

  // Interactor
  this->Internals->init("interactor.axis", false); // bool
  this->Internals->init("interactor.trackball", false); // bool
  this->Internals->init("interactor.invert-zoom", false); // bool*/
};

//----------------------------------------------------------------------------
options::~options()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
options::options(const options& opt)
  : Internals(new options::internals)
{
//  this->Internals->Options = opt.Internals->Options;
  this->Internals->OptionStruct = opt.Internals->OptionStruct;
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt) noexcept
{
//  this->Internals->Options = opt.Internals->Options;
  this->Internals->OptionStruct = opt.Internals->OptionStruct;
  return *this;
}

//----------------------------------------------------------------------------
options::options(options&& other) noexcept
{
  this->Internals = other.Internals;
  other.Internals = nullptr;
}

//----------------------------------------------------------------------------
options& options::operator=(options&& other) noexcept
{
  if (this != &other)
  {
    delete this->Internals;
    this->Internals = other.Internals;
    other.Internals = nullptr;
  }
  return *this;
}

//----------------------------------------------------------------------------
void options::setVariant(const std::string& name, option_variant_t value)
{
  this->Internals->setVariant(name, value);
}

//----------------------------------------------------------------------------
option_variant_t options::getVariant(const std::string& name)
{
  return this->Internals->getVariant(name);
}

//----------------------------------------------------------------------------
void options::setString(const std::string& name, std::string value)
{
  this->Internals->setString(name, value);
}

//----------------------------------------------------------------------------
std::string options::getString(const std::string& name)
{
  return this->Internals->getString(name);
}
/*
//----------------------------------------------------------------------------
options& options::set(const std::string& name, bool value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, int value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, double value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::string& value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const char* value)
{
  this->Internals->set(name, std::string(value));
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::vector<int>& values)
{
  this->Internals->set(name, values);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::vector<double>& values)
{
  this->Internals->set(name, values);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, std::initializer_list<int> values)
{
  this->Internals->set(name, std::vector<int>(values));
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, std::initializer_list<double> values)
{
  this->Internals->set(name, std::vector<double>(values));
  return *this;
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, bool& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, int& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, double& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::string& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::vector<int>& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::vector<double>& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
bool options::getAsBool(const std::string& name) const
{
  return this->Internals->get<bool>(name);
}

//----------------------------------------------------------------------------
int options::getAsInt(const std::string& name) const
{
  return this->Internals->get<int>(name);
}

//----------------------------------------------------------------------------
double options::getAsDouble(const std::string& name) const
{
  return this->Internals->get<double>(name);
}

//----------------------------------------------------------------------------
std::string options::getAsString(const std::string& name) const
{
  return this->Internals->get<std::string>(name);
}

//----------------------------------------------------------------------------
std::vector<int> options::getAsIntVector(const std::string& name) const
{
  return this->Internals->get<std::vector<int>>(name);
}

//----------------------------------------------------------------------------
std::vector<double> options::getAsDoubleVector(const std::string& name) const
{
  return this->Internals->get<std::vector<double>>(name);
}

//----------------------------------------------------------------------------
bool& options::getAsBoolRef(const std::string& name)
{
  return this->Internals->getRef<bool>(name);
}

//----------------------------------------------------------------------------
int& options::getAsIntRef(const std::string& name)
{
  return this->Internals->getRef<int>(name);
}

//----------------------------------------------------------------------------
double& options::getAsDoubleRef(const std::string& name)
{
  return this->Internals->getRef<double>(name);
}

//----------------------------------------------------------------------------
std::string& options::getAsStringRef(const std::string& name)
{
  return this->Internals->getRef<std::string>(name);
}

//----------------------------------------------------------------------------
std::vector<int>& options::getAsIntVectorRef(const std::string& name)
{
  return this->Internals->getRef<std::vector<int>>(name);
}

//----------------------------------------------------------------------------
std::vector<double>& options::getAsDoubleVectorRef(const std::string& name)
{
  return this->Internals->getRef<std::vector<double>>(name);
}

//----------------------------------------------------------------------------
options& options::toggle(const std::string& name)
{
  this->Internals->set<bool>(name, !this->Internals->get<bool>(name));
  return *this;
}

//----------------------------------------------------------------------------
options& options::toggleNew(const std::string& name)
{
  this->Internals->setVariant(name, !std::get<bool>(this->Internals->getVariant(name)));
  return *this;
}
*/
//----------------------------------------------------------------------------
bool options::isSame(const options& other, const std::string& name) const
{
  try
  {
    return this->Internals->getVariant(name) == other.Internals->getVariant(name);
  }
  catch (const std::out_of_range&)
  {
    // TODO error mgt
    std::string error = "Options " + name + " does not exist";
    log::error(error);
    throw options::inexistent_exception(error + "\n");
  }
}

//----------------------------------------------------------------------------
options& options::copy(const options& from, const std::string& name)
{
  try
  {
    this->Internals->setVariant(name, from.Internals->getVariant(name));
  }
  catch (const std::out_of_range&)
  {
    // TODO error mgt
    std::string error = "Options " + name + " does not exist";
    log::error(error);
    throw options::inexistent_exception(error + "\n");
  }
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getNames()
{
  return this->Internals->getNames();
}

//----------------------------------------------------------------------------
std::pair<std::string, unsigned int> options::getClosestOption(const std::string& option) const
{
  std::vector<std::string> names = this->Internals->getNames();
  if (std::find(names.begin(), names.end(), option) != names.end())
  {
    return { option, 0 };
  }

  std::pair<std::string, int> ret = { "", std::numeric_limits<int>::max() };

  for (const auto& name : names)
  {
    int distance = utils::textDistance(name, option);
    if (distance < ret.second)
    {
      ret = { name, distance };
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
options::incompatible_exception::incompatible_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
options::inexistent_exception::inexistent_exception(const std::string& what)
  : exception(what)
{
}

options_struct& options::getStruct(){return this->Internals->OptionStruct;}
const options_struct& options::getConstStruct() const{return this->Internals->OptionStruct;}

}
