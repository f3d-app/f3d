#ifndef f3d_options_h
#define f3d_options_h

#include "exception.h"
#include "export.h"
#include "types.h"

#include <string>
#include <vector>
#include <variant>
#include <array>

namespace f3d
{

// TODO GENERATE
struct options_struct {
  struct scene_t {
  std::string up_direction = "+Y";
  struct animation_t {
    bool autoplay = false;
    int index = 0;
    double speed_factor = 1.0;
    double time = 0.0;
    double frame_rate = 60.0;
  };
  animation_t animation;
  struct camera_t {
    int index = -1;
    bool orthographic = false;
  };
  camera_t camera;
  };
 scene_t scene;

 struct render_t {
   bool show_edges = false;
   double line_width = 1.0;
   double point_size = 10.0;
   std::string backface_type = "default";
   struct grid_t {
     bool enable = false;
     bool absolute = false;
     double unit = 0.0;
     int subdivisions = 10;
     std::vector<double> color = {0.0, 0.0, 0.0};
   };
   grid_t grid;
   struct raytracing_t {
     bool enable = false;
     bool denoise = false;
     int samples = 5;
   };
   raytracing_t raytracing;
   struct effect_t {
     bool translucency_support = false;
     bool anti_aliasing = false;
     bool ambient_occlusion = false;
     bool tone_mapping = false;
     std::string final_shader;
   };
   effect_t effect;
   struct hdri_t {
     std::string file;
     bool ambient = false;
   };
   hdri_t hdri;
   struct background_t {
     std::vector<double> color{ 0.2, 0.2, 0.2 };
     bool skybox = false;
     bool blur = false;
     double blur_coc = 20.0;
   };
   background_t background;
   struct light_t {
     double intensity = 1.0;
   };
   light_t light;
 };
 render_t render;

 struct ui_t {
   bool scalar_bar = false;
   bool filename = false;
   std::string filename_info;
   bool fps = false;
   bool cheatsheet = false;
   bool dropzone = false;
   std::string dropzone_info;
   bool metadata = false;
   std::string font_file;
   bool loader_progress = false;
   bool animation_progress = false;
 };
 ui_t ui;

 struct model_t {
   struct matcap_t {
     std::string texture;
   };
   matcap_t matcap;
   struct color_t {
     double opacity = 1.0;
     std::vector<double> rgb = { 1.0, 1.0, 1.0 };
     std::string texture;
   };
   color_t color;
   struct emissive_t {
     std::vector<double> factor = { 1.0, 1.0, 1.0 };
     std::string texture;
   };
   emissive_t emissive;
   struct normal_t {
     double scale = 1.0;
     std::string texture;
   };
   normal_t normal;
   struct material_t {
     double metallic = 0.0;
     double roughness = 0.3;
     std::string texture;
   };
   material_t material;
   struct scivis_t {
     bool cells = false;
     std::string array_name = "f3d_reserved";
     int component = -1;
     std::vector<double> colormap = std::vector<double>{
      0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 };
     std::vector<double> range = std::vector<double>{ 0 };
   };
   scivis_t scivis;
   struct point_sprites_t {
     bool enable = false;
     std::string type = "sphere";
   };
   point_sprites_t point_sprites;
   struct volume_t {
     bool enable = false;
     bool inverse = false;
   };
   volume_t volume;
 };
 model_t model;

 struct interactor_t {
   bool axis = false;
   bool trackball = false;
   bool invert_zoom = false;
 };
 interactor_t interactor;

};

/**
 * @class   options
 * @brief   Class used to control the different options
 *
 * A class to control the different options of f3d.
 * See the README_libf3d.md for the full listing of options
 */
class F3D_EXPORT options
{
public:
  ///@{ @name Constructors
  /**
   * Default/Copy/move constructors/operators.
   */
  options();
  ~options();
  options(const options& opt);
  options& operator=(const options& opt) noexcept;
  options(options&& other) noexcept;
  options& operator=(options&& other) noexcept;
  ///@}

  void setVariant(const std::string& name, option_variant_t value);
  option_variant_t getVariant(const std::string& name);
  void setString(const std::string& name, std::string value);
  std::string getString(const std::string& name);

  ///@{ @name Setters
  /**
   * Setters for all supported types.
   */
/*  options& set(const std::string& name, bool value);
  options& set(const std::string& name, int value);
  options& set(const std::string& name, double value);
  options& set(const std::string& name, const std::string& value);
  options& set(const std::string& name, const char* value);
  options& set(const std::string& name, const std::vector<int>& values);
  options& set(const std::string& name, const std::vector<double>& values);
  options& set(const std::string& name, std::initializer_list<int> values);
  options& set(const std::string& name, std::initializer_list<double> values);*/
  ///@}

  ///@{ @name Reference Getters
  /**
   * Copy the option value into the provided reference, for all supported types.
   */
/*  void get(const std::string& name, bool& value) const;
  void get(const std::string& name, int& value) const;
  void get(const std::string& name, double& value) const;
  void get(const std::string& name, std::string& value) const;
  void get(const std::string& name, std::vector<int>& value) const;
  void get(const std::string& name, std::vector<double>& value) const;*/
  ///@}

  ///@{ @name Explicit Copy Getters
  /**
   * Explicit getters for all supported types.
   */
/*  bool getAsBool(const std::string& name) const;
  int getAsInt(const std::string& name) const;
  double getAsDouble(const std::string& name) const;
  std::string getAsString(const std::string& name) const;
  std::vector<int> getAsIntVector(const std::string& name) const;
  std::vector<double> getAsDoubleVector(const std::string& name) const;*/
  ///@}

  ///@{ @name Explicit Reference Getters
  /**
   * Explicit getters to actual reference to the options variable, for all supported types.
   * Modifying the returned reference will modify the option.
   * Throw an options::incompatible_exception if the type is not compatible with the option.
   * Throw an options::inexistent_exception if option does not exist.
   */
/*  bool& getAsBoolRef(const std::string& name);
  int& getAsIntRef(const std::string& name);
  double& getAsDoubleRef(const std::string& name);
  std::string& getAsStringRef(const std::string& name);
  std::vector<int>& getAsIntVectorRef(const std::string& name);
  std::vector<double>& getAsDoubleVectorRef(const std::string& name);*/
  ///@}

  /**
   * A boolean option specific method to toggle it.
   */
/*  options& toggle(const std::string& name);
  options& toggleNew(const std::string& name);*/

  /**
   * Compare an option between this and a provided other.
   * Return true is they are the same value, false otherwise.
   * Throw an options::inexistent_exception if option does not exist.
   */
  bool isSame(const options& other, const std::string& name) const;

  /**
   * Copy the value of an option from this to the provided other.
   * Throw an options::inexistent_exception if option does not exist.
   */
  options& copy(const options& other, const std::string& name);

  /**
   * Get all available option names.
   */
  std::vector<std::string> getNames();

  /**
   * Get the closest option name and its Levenshtein distance.
   */
  std::pair<std::string, unsigned int> getClosestOption(const std::string& option) const;

  /**
   * An exception that can be thrown by the options
   * when a provided option type is incompatible with
   * its internal type.
   */
  struct incompatible_exception : public exception
  {
    explicit incompatible_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the options
   * when a provided option does not exist.
   */
  struct inexistent_exception : public exception
  {
    explicit inexistent_exception(const std::string& what = "");
  };

  options_struct& getStruct();
  const options_struct& getConstStruct() const;

private:
  class internals;
  internals* Internals;
};
}

#endif
