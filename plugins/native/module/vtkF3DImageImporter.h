/**
 * @class   vtkF3DImageImporter
 * @brief   Import an image file as a textured quad
 *
 * This importer reads 2D image files using vtkImageReader2Factory and displays them as a textured quad
 * sized to the image dimensions. Supports both file-based and stream-based reading.
 */

#ifndef vtkF3DImageImporter_h
#define vtkF3DImageImporter_h

#include <vtkF3DImporter.h>

class vtkF3DImageImporter : public vtkF3DImporter
{
public:
  static vtkF3DImageImporter* New();
  vtkTypeMacro(vtkF3DImageImporter, vtkF3DImporter);

protected:
  vtkF3DImageImporter() = default;
  ~vtkF3DImageImporter() override = default;

  void ImportActors(vtkRenderer*) override;

private:
  vtkF3DImageImporter(const vtkF3DImageImporter&) = delete;
  void operator=(const vtkF3DImageImporter&) = delete;
};

#endif
