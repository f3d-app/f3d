/**
 * @class   F3DExodusIIReader
 * @brief   The ExodusII reader class
 *
 */

#ifndef F3DExodusIIReader_h
#define F3DExodusIIReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkExodusIIReader.h>
#endif

#include <regex>

class F3DExodusIIReader : public F3DReader
{
public:
  F3DExodusIIReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "ExodusIIReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "ExodusII files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".exo", ".ex2", ".e" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/vnd.exodus" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkExodusIIReader> reader = vtkSmartPointer<vtkExodusIIReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->UpdateInformation();
    reader->SetAllArrayStatus(vtkExodusIIReader::NODAL, 1);
    reader->SetAllArrayStatus(vtkExodusIIReader::ELEM_BLOCK, 1);
    return reader;
  }
#endif
};

#endif
