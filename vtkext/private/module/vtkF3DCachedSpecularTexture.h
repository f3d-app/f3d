/**
 * @class   vtkF3DCachedSpecularTexture
 * @brief   create a prefiltered specular texture from a vtm file
 */

#ifndef vtkF3DCachedSpecularTexture_h
#define vtkF3DCachedSpecularTexture_h

#include "vtkPBRPrefilterTexture.h"

class vtkF3DCachedSpecularTexture : public vtkPBRPrefilterTexture
{
public:
  static vtkF3DCachedSpecularTexture* New();
  vtkTypeMacro(vtkF3DCachedSpecularTexture, vtkPBRPrefilterTexture);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the image file name.
   */
  vtkSetMacro(FileName, std::string);

  /**
   * Implement base class method.
   */
  void Load(vtkRenderer*) override;

  ///@{
  /**
   * Turn on or off cache mechanism
   * Default is off
   */
  vtkSetMacro(UseCache, bool);
  vtkBooleanMacro(UseCache, bool);
  ///@}

protected:
  vtkF3DCachedSpecularTexture() = default;
  ~vtkF3DCachedSpecularTexture() override = default;

  std::string FileName;
  bool UseCache = false;

private:
  vtkF3DCachedSpecularTexture(const vtkF3DCachedSpecularTexture&) = delete;
  void operator=(const vtkF3DCachedSpecularTexture&) = delete;
};

#endif
