/**
 * @class   F3DNrrdReader
 * @brief   The Nrrd reader class
 *
 */

#ifndef F3DNrrdReader_h
#define F3DNrrdReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkNrrdReader.h>
#endif

class F3DNrrdReader : public F3DReader
{
public:
  F3DNrrdReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "NrrdReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "Nrrd files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".nrrd", ".nhdr" };
    return ext;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkNrrdReader> reader = vtkSmartPointer<vtkNrrdReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
