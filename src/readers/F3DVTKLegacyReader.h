/**
 * @class   F3DVTKLegacyReader
 * @brief   The VTK Legacy reader class
 *
 */

#ifndef F3DVTKLegacyReader_h
#define F3DVTKLegacyReader_h

#include "F3DReaderFactory.h"

#include <vtkPDataSetReader.h>

class F3DVTKLegacyReader : public F3DReader
{
public:
  F3DVTKLegacyReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "VTKLegacyReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "VTK Legacy files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".vtk" };
    return ext;
  }

  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkPDataSetReader> reader = vtkSmartPointer<vtkPDataSetReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
};

#endif
