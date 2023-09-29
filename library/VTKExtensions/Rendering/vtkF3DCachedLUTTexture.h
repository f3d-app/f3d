/**
 * @class   vtkF3DCachedLUTTexture
 * @brief   create a LUT texture from a vti file
 */

#ifndef vtkF3DCachedLUTTexture_h
#define vtkF3DCachedLUTTexture_h

#include "vtkPBRLUTTexture.h"

class vtkF3DCachedLUTTexture : public vtkPBRLUTTexture
{
public:
  static vtkF3DCachedLUTTexture* New();
  vtkTypeMacro(vtkF3DCachedLUTTexture, vtkPBRLUTTexture);
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
  vtkF3DCachedLUTTexture() = default;
  ~vtkF3DCachedLUTTexture() override = default;

  std::string FileName;
  bool UseCache = false;

private:
  vtkF3DCachedLUTTexture(const vtkF3DCachedLUTTexture&) = delete;
  void operator=(const vtkF3DCachedLUTTexture&) = delete;
};

#endif
