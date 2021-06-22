/**
 * @class   F3DSTLReader
 * @brief   The STL reader class
 *
 */

#ifndef F3DSTLReader_h
#define F3DSTLReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkSTLReader.h>
#include <vtkVersion.h>
#endif

  class F3DSTLReader : public F3DReader
{
public:
  F3DSTLReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "STLReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "STL files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".stl" };
    return ext;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200616)
    reader->MergingOff();
#endif
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
