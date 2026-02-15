/**
 * @class   vtkF3DImageImporter
 * @brief   Import an image file as a textured quad
 *
 * This importer reads 2D image files using vtkImageReader2Factory and displays them as a textured
 * quad sized to the image dimensions. Supports both file-based and stream-based reading.
 */

#ifndef vtkF3DImageImporter_h
#define vtkF3DImageImporter_h

#include <vtkF3DImporter.h>

class vtkF3DImageImporter : public vtkF3DImporter
{
public:
  static vtkF3DImageImporter* New();
  vtkTypeMacro(vtkF3DImageImporter, vtkF3DImporter);

  ///@{
  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read as a stream.
   *
   * hint is provided to be passed as an ImageHint
   */
  static bool CanReadFile(vtkResourceStream* stream, std::string& hint);
  static bool CanReadFile(vtkResourceStream* stream);
  ///@}

  /**
   * Set the image hint create an image reader from
   */
  vtkSetMacro(ImageHint, std::string);

protected:
  vtkF3DImageImporter() = default;
  ~vtkF3DImageImporter() override = default;

  void ImportActors(vtkRenderer*) override;

private:
  vtkF3DImageImporter(const vtkF3DImageImporter&) = delete;
  void operator=(const vtkF3DImageImporter&) = delete;

  std::string ImageHint;
};

#endif
