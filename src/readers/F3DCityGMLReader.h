/**
 * @class   F3DCityGMLReader
 * @brief   The CityGML reader class
 *
 */

#ifndef F3DCityGMLReader_h
#define F3DCityGMLReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkCityGMLReader.h>
#endif

class F3DCityGMLReader : public F3DReader
{
public:
  F3DCityGMLReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "CityGMLReader"; }

  /**
   *  Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "CityGML files reader"; }

  /**
   *  Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".gml" };
    return ext;
  }

#ifndef F3D_NO_VTK
  /**
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkCityGMLReader> reader = vtkSmartPointer<vtkCityGMLReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
