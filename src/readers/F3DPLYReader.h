/**
 * @class   F3DPLYReader
 * @brief   The PLY reader class
 *
 */

#ifndef F3DPLYReader_h
#define F3DPLYReader_h

#include "F3DReaderFactory.h"

#include <vtkPLYReader.h>

class F3DPLYReader : public F3DReader
{
public:
  F3DPLYReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "PLYReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "PLY files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".ply" };
    return ext;
  }

  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
};

#endif
