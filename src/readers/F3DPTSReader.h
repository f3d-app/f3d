/**
 * @class   F3DPTSReader
 * @brief   The PTS reader class
 *
 */

#ifndef F3DPTSReader_h
#define F3DPTSReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkPTSReader.h>
#endif

class F3DPTSReader : public F3DReader
{
public:
  F3DPTSReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "PTSReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "PTS files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".pts" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/vnd.pts" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkPTSReader> reader = vtkSmartPointer<vtkPTSReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
