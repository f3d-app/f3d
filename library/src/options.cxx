#include "options.h"
#include "options_struct.h"
#include "options_struct_internals.h"

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

/*  void setVariant(const std::string& name, option_variant_t value)
  {
    // TODO GENERATE
    if (name == "scene.up_direction")
    {
      this->OptionsStruct.scene.up_direction = std::get<std::string>(value);
    }
    else if (name == "scene.animation.autoplay")
    {
      this->OptionsStruct.scene.animation.autoplay = std::get<bool>(value);
    }
    else if (name == "scene.animation.index")
    {
      this->OptionsStruct.scene.animation.index = std::get<int>(value);
    }
    else if (name == "scene.animation.speed_factor")
    {
      this->OptionsStruct.scene.animation.speed_factor = std::get<double>(value);
    }
    else if (name == "scene.animation.time")
    {
      this->OptionsStruct.scene.animation.time = std::get<double>(value);
    }
    else if (name == "scene.animation.frame_rate")
    {
      this->OptionsStruct.scene.animation.frame_rate = std::get<double>(value);
    }
    else if (name == "scene.camera.index")
    {
      this->OptionsStruct.scene.camera.index = std::get<int>(value);
    }
    else if (name == "scene.camera.orthographic")
    {
      this->OptionsStruct.scene.camera.orthographic = std::get<bool>(value);
    }

    else if (name == "render.show_edges")
    {
      this->OptionsStruct.render.show_edges = std::get<bool>(value);
    }
    else if (name == "render.line_width")
    {
      this->OptionsStruct.render.line_width = std::get<double>(value);
    }
    else if (name == "render.point_size")
    {
      this->OptionsStruct.render.point_size = std::get<double>(value);
    }
    else if (name == "render.backface_type")
    {
      this->OptionsStruct.render.backface_type = std::get<std::string>(value);
    }
    else if (name == "render.grid.enable")
    {
      this->OptionsStruct.render.grid.enable = std::get<bool>(value);
    }
    else if (name == "render.grid.absolute")
    {
      this->OptionsStruct.render.grid.absolute = std::get<bool>(value);
    }
    else if (name == "render.grid.unit")
    {
      this->OptionsStruct.render.grid.unit = std::get<double>(value);
    }
    else if (name == "render.grid.subdivisions")
    {
      this->OptionsStruct.render.grid.subdivisions = std::get<int>(value);
    }
    else if (name == "render.grid.color")
    {
      this->OptionsStruct.render.grid.color = std::get<std::vector<double>>(value);
    }
    else if (name == "render.raytracing.enable")
    {
      this->OptionsStruct.render.raytracing.enable = std::get<bool>(value);
    }
    else if (name == "render.raytracing.denoise")
    {
      this->OptionsStruct.render.raytracing.denoise = std::get<bool>(value);
    }
    else if (name == "render.raytracing.samples")
    {
      this->OptionsStruct.render.raytracing.samples = std::get<int>(value);
    }
    else if (name == "render.effect.translucency_support")
    {
      this->OptionsStruct.render.effect.translucency_support = std::get<bool>(value);
    }
    else if (name == "render.effect.anti_aliasing")
    {
      this->OptionsStruct.render.effect.anti_aliasing = std::get<bool>(value);
    }
    else if (name == "render.effect.ambient_occlusion")
    {
      this->OptionsStruct.render.effect.ambient_occlusion = std::get<bool>(value);
    }
    else if (name == "render.effect.tone_mapping")
    {
      this->OptionsStruct.render.effect.tone_mapping = std::get<bool>(value);
    }
    else if (name == "render.effect.final_shader")
    {
      this->OptionsStruct.render.effect.final_shader = std::get<std::string>(value);
    }
    else if (name == "render.hdri.file")
    {
      this->OptionsStruct.render.hdri.file = std::get<std::string>(value);
    }
    else if (name == "render.hdri.ambient")
    {
      this->OptionsStruct.render.hdri.ambient = std::get<bool>(value);
    }
    else if (name == "render.background.color")
    {
      this->OptionsStruct.render.background.color = std::get<std::vector<double>>(value);
    }
    else if (name == "render.background.skybox")
    {
      this->OptionsStruct.render.background.skybox = std::get<bool>(value);
    }
    else if (name == "render.background.blur")
    {
      this->OptionsStruct.render.background.blur = std::get<bool>(value);
    }
    else if (name == "render.background.blur_coc")
    {
      this->OptionsStruct.render.background.blur_coc = std::get<double>(value);
    }
    else if (name == "render.light.intensity")
    {
      this->OptionsStruct.render.light.intensity = std::get<double>(value);
    }

    else if (name == "ui.scalar_bar")
    {
      this->OptionsStruct.ui.scalar_bar = std::get<bool>(value);
    }
    else if (name == "ui.filename")
    {
      this->OptionsStruct.ui.filename = std::get<bool>(value);
    }
    else if (name == "ui.filename_info")
    {
      this->OptionsStruct.ui.filename_info = std::get<std::string>(value);
    }
    else if (name == "ui.fps")
    {
      this->OptionsStruct.ui.fps = std::get<bool>(value);
    }
    else if (name == "ui.cheatsheet")
    {
      this->OptionsStruct.ui.cheatsheet = std::get<bool>(value);
    }
    else if (name == "ui.dropzone")
    {
      this->OptionsStruct.ui.dropzone = std::get<bool>(value);
    }
    else if (name == "ui.dropzone_info")
    {
      this->OptionsStruct.ui.dropzone_info = std::get<std::string>(value);
    }
    else if (name == "ui.metadata")
    {
      this->OptionsStruct.ui.metadata = std::get<bool>(value);
    }
    else if (name == "ui.font_file")
    {
      this->OptionsStruct.ui.font_file = std::get<std::string>(value);
    }
    else if (name == "ui.loader_progress")
    {
      this->OptionsStruct.ui.loader_progress = std::get<bool>(value);
    }
    else if (name == "ui.animation_progress")
    {
      this->OptionsStruct.ui.animation_progress = std::get<bool>(value);
    }

    else if (name == "model.matcap.texture")
    {
      this->OptionsStruct.model.matcap.texture = std::get<std::string>(value);
    }
    else if (name == "model.color.opacity")
    {
      this->OptionsStruct.model.color.opacity = std::get<double>(value);
    }
    else if (name == "model.color.rgb")
    {
      this->OptionsStruct.model.color.rgb = std::get<std::vector<double>>(value);
    }
    else if (name == "model.color.texture")
    {
      this->OptionsStruct.model.color.texture = std::get<std::string>(value);
    }
    else if (name == "model.emissive.factor")
    {
      this->OptionsStruct.model.emissive.factor = std::get<std::vector<double>>(value);
    }
    else if (name == "model.emissive.texture")
    {
      this->OptionsStruct.model.emissive.texture = std::get<std::string>(value);
    }
    else if (name == "model.normal.scale")
    {
      this->OptionsStruct.model.normal.scale = std::get<double>(value);
    }
    else if (name == "model.normal.texture")
    {
      this->OptionsStruct.model.normal.texture = std::get<std::string>(value);
    }
    else if (name == "model.material.metallic")
    {
      this->OptionsStruct.model.material.metallic = std::get<double>(value);
    }
    else if (name == "model.material.roughness")
    {
      this->OptionsStruct.model.material.roughness = std::get<double>(value);
    }
    else if (name == "model.material.texture")
    {
      this->OptionsStruct.model.material.texture = std::get<std::string>(value);
    }
    else if (name == "model.scivis.cells")
    {
      this->OptionsStruct.model.scivis.cells = std::get<bool>(value);
    }
    else if (name == "model.scivis.array_name")
    {
      this->OptionsStruct.model.scivis.array_name = std::get<std::string>(value);
    }
    else if (name == "model.scivis.component")
    {
      this->OptionsStruct.model.scivis.component = std::get<int>(value);
    }
    else if (name == "model.scivis.colormap")
    {
      this->OptionsStruct.model.scivis.colormap = std::get<std::vector<double>>(value);
    }
    else if (name == "model.scivis.range")
    {
      this->OptionsStruct.model.scivis.range = std::get<std::vector<double>>(value);
    }
    else if (name == "model.point_sprites.enable")
    {
      this->OptionsStruct.model.point_sprites.enable = std::get<bool>(value);
    }
    else if (name == "model.point_sprites.type")
    {
      this->OptionsStruct.model.point_sprites.type = std::get<std::string>(value);
    }
    else if (name == "model.volume.enable")
    {
      this->OptionsStruct.model.volume.enable = std::get<bool>(value);
    }
    else if (name == "model.volume.inverse")
    {
      this->OptionsStruct.model.volume.inverse = std::get<bool>(value);
    }

    else if (name == "interactor.axis")
    {
      this->OptionsStruct.interactor.axis = std::get<bool>(value);
    }
    else if (name == "interactor.trackball")
    {
      this->OptionsStruct.interactor.trackball = std::get<bool>(value);
    }
    else if (name == "interactor.invert_zoom")
    {
      this->OptionsStruct.interactor.invert_zoom = std::get<bool>(value);
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
      var = this->OptionsStruct.scene.up_direction;
    }
    else if (name == "scene.animation.autoplay")
    {
      var = this->OptionsStruct.scene.animation.autoplay;
    }
    else if (name == "scene.animation.index")
    {
      var = this->OptionsStruct.scene.animation.index;
    }
    else if (name == "scene.animation.speed_factor")
    {
      var = this->OptionsStruct.scene.animation.speed_factor;
    }
    else if (name == "scene.animation.time")
    {
      var = this->OptionsStruct.scene.animation.time;
    }
    else if (name == "scene.animation.frame_rate")
    {
      var = this->OptionsStruct.scene.animation.frame_rate;
    }
    else if (name == "scene.camera.index")
    {
      var = this->OptionsStruct.scene.camera.index;
    }
    else if (name == "scene.camera.orthographic")
    {
      var = this->OptionsStruct.scene.camera.orthographic;
    }

    else if (name == "render.show_edges")
    {
      var = this->OptionsStruct.render.show_edges;
    }
    else if (name == "render.line_width")
    {
      var = this->OptionsStruct.render.line_width;
    }
    else if (name == "render.point_size")
    {
      var = this->OptionsStruct.render.point_size;
    }
    else if (name == "render.backface_type")
    {
      var = this->OptionsStruct.render.backface_type;
    }
    else if (name == "render.grid.enable")
    {
      var = this->OptionsStruct.render.grid.enable;
    }
    else if (name == "render.grid.absolute")
    {
      var = this->OptionsStruct.render.grid.absolute;
    }
    else if (name == "render.grid.unit")
    {
      var = this->OptionsStruct.render.grid.unit;
    }
    else if (name == "render.grid.subdivisions")
    {
      var = this->OptionsStruct.render.grid.subdivisions;
    }
    else if (name == "render.grid.color")
    {
      var = this->OptionsStruct.render.grid.color;
    }
    else if (name == "render.raytracing.enable")
    {
      var = this->OptionsStruct.render.raytracing.enable;
    }
    else if (name == "render.raytracing.denoise")
    {
      var = this->OptionsStruct.render.raytracing.denoise;
    }
    else if (name == "render.raytracing.samples")
    {
      var = this->OptionsStruct.render.raytracing.samples;
    }
    else if (name == "render.effect.translucency_support")
    {
      var = this->OptionsStruct.render.effect.translucency_support;
    }
    else if (name == "render.effect.anti_aliasing")
    {
      var = this->OptionsStruct.render.effect.anti_aliasing;
    }
    else if (name == "render.effect.ambient_occlusion")
    {
      var = this->OptionsStruct.render.effect.ambient_occlusion;
    }
    else if (name == "render.effect.tone_mapping")
    {
      var = this->OptionsStruct.render.effect.tone_mapping;
    }
    else if (name == "render.effect.final_shader")
    {
      var = this->OptionsStruct.render.effect.final_shader;
    }
    else if (name == "render.hdri.file")
    {
      var = this->OptionsStruct.render.hdri.file;
    }
    else if (name == "render.hdri.ambient")
    {
      var = this->OptionsStruct.render.hdri.ambient;
    }
    else if (name == "render.background.color")
    {
      var = this->OptionsStruct.render.background.color;
    }
    else if (name == "render.background.skybox")
    {
      var = this->OptionsStruct.render.background.skybox;
    }
    else if (name == "render.background.blur")
    {
      var = this->OptionsStruct.render.background.blur;
    }
    else if (name == "render.background.blur_coc")
    {
      var = this->OptionsStruct.render.background.blur_coc;
    }
    else if (name == "render.light.intensity")
    {
      var = this->OptionsStruct.render.light.intensity;
    }

    else if (name == "ui.scalar_bar")
    {
      var = this->OptionsStruct.ui.scalar_bar;
    }
    else if (name == "ui.filename")
    {
      var = this->OptionsStruct.ui.filename;
    }
    else if (name == "ui.filename_info")
    {
      var = this->OptionsStruct.ui.filename_info;
    }
    else if (name == "ui.fps")
    {
      var = this->OptionsStruct.ui.fps;
    }
    else if (name == "ui.cheatsheet")
    {
      var = this->OptionsStruct.ui.cheatsheet;
    }
    else if (name == "ui.dropzone")
    {
      var = this->OptionsStruct.ui.dropzone;
    }
    else if (name == "ui.dropzone_info")
    {
      var = this->OptionsStruct.ui.dropzone_info;
    }
    else if (name == "ui.metadata")
    {
      var = this->OptionsStruct.ui.metadata;
    }
    else if (name == "ui.font_file")
    {
      var = this->OptionsStruct.ui.font_file;
    }
    else if (name == "ui.loader_progress")
    {
      var = this->OptionsStruct.ui.loader_progress;
    }
    else if (name == "ui.animation_progress")
    {
      var = this->OptionsStruct.ui.animation_progress;
    }

    else if (name == "model.matcap.texture")
    {
      var = this->OptionsStruct.model.matcap.texture;
    }
    else if (name == "model.color.opacity")
    {
      var = this->OptionsStruct.model.color.opacity;
    }
    else if (name == "model.color.rgb")
    {
      var = this->OptionsStruct.model.color.rgb;
    }
    else if (name == "model.color.texture")
    {
      var = this->OptionsStruct.model.color.texture;
    }
    else if (name == "model.emissive.factor")
    {
      var = this->OptionsStruct.model.emissive.factor;
    }
    else if (name == "model.emissive.texture")
    {
      var = this->OptionsStruct.model.emissive.texture;
    }
    else if (name == "model.normal.scale")
    {
      var = this->OptionsStruct.model.normal.scale;
    }
    else if (name == "model.normal.texture")
    {
      var = this->OptionsStruct.model.normal.texture;
    }
    else if (name == "model.material.metallic")
    {
      var = this->OptionsStruct.model.material.metallic;
    }
    else if (name == "model.material.roughness")
    {
      var = this->OptionsStruct.model.material.roughness;
    }
    else if (name == "model.material.texture")
    {
      var = this->OptionsStruct.model.material.texture;
    }
    else if (name == "model.scivis.cells")
    {
      var = this->OptionsStruct.model.scivis.cells;
    }
    else if (name == "model.scivis.array_name")
    {
      var = this->OptionsStruct.model.scivis.array_name;
    }
    else if (name == "model.scivis.component")
    {
      var = this->OptionsStruct.model.scivis.component;
    }
    else if (name == "model.scivis.colormap")
    {
      var = this->OptionsStruct.model.scivis.colormap;
    }
    else if (name == "model.scivis.range")
    {
      var = this->OptionsStruct.model.scivis.range;
    }
    else if (name == "model.point_sprites.enable")
    {
      var = this->OptionsStruct.model.point_sprites.enable;
    }
    else if (name == "model.point_sprites.type")
    {
      var = this->OptionsStruct.model.point_sprites.type;
    }
    else if (name == "model.volume.enable")
    {
      var = this->OptionsStruct.model.volume.enable;
    }
    else if (name == "model.volume.inverse")
    {
      var = this->OptionsStruct.model.volume.inverse;
    }

    else if (name == "interactor.axis")
    {
      var = this->OptionsStruct.interactor.axis;
    }
    else if (name == "interactor.trackball")
    {
      var = this->OptionsStruct.interactor.trackball;
    }
    else if (name == "interactor.invert_zoom")
    {
      var = this->OptionsStruct.interactor.invert_zoom;
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

     "model.point_sprites.type",

     "model.volume.enable",

     "model.volume.inverse",

     "interactor.axis",

     "interactor.trackball",

     "interactor.invert_zoom",

    };
    return vec;
  }*/

  void setString(const std::string& name, std::string value)
  {
    option_variant_t var = options_struct_internals::get(this->OptionsStruct, name);
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
    options_struct_internals::set(this->OptionsStruct, name, var);
  }

  std::string getString(const std::string& name)
  {
    option_variant_t var = options_struct_internals::get(this->OptionsStruct, name);
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

  options_struct OptionsStruct;
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
  this->Internals->OptionsStruct = opt.Internals->OptionsStruct;
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt) noexcept
{
//  this->Internals->Options = opt.Internals->Options;
  this->Internals->OptionsStruct = opt.Internals->OptionsStruct;
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
  options_struct_internals::set(this->Internals->OptionsStruct, name, value);
}

//----------------------------------------------------------------------------
option_variant_t options::getVariant(const std::string& name)
{
  return options_struct_internals::get(this->Internals->OptionsStruct, name);
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
    return options_struct_internals::get(this->Internals->OptionsStruct, name) == options_struct_internals::get(other.Internals->OptionsStruct, name);
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
    options_struct_internals::set(this->Internals->OptionsStruct, name, options_struct_internals::get(from.Internals->OptionsStruct, name));
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
  return options_struct_internals::getNames();
}

//----------------------------------------------------------------------------
std::pair<std::string, unsigned int> options::getClosestOption(const std::string& option) const
{
  std::vector<std::string> names = options_struct_internals::getNames();
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

options_struct& options::getStruct(){return this->Internals->OptionsStruct;}
const options_struct& options::getConstStruct() const{return this->Internals->OptionsStruct;}

}
