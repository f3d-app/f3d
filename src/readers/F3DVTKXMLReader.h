/**
 * @class   F3DVTKXMLReader
 * @brief   The VTK XML reader class
 *
 */

#ifndef F3DVTKXMLReader_h
#define F3DVTKXMLReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkXMLGenericDataObjectReader.h>
#endif

class F3DVTKXMLReader : public F3DReader
{
public:
  F3DVTKXMLReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "VTKXMLReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "VTK XML files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".vtp", ".vtu", ".vtr", ".vti", ".vts", ".vtm" };
    return ext;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkXMLGenericDataObjectReader> reader =
      vtkSmartPointer<vtkXMLGenericDataObjectReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
