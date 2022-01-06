/**
 * @class   F3DReader
 * @brief   The basis reader class
 *
 * F3DReader is the basic class for every file format readers.
 * It must provide information about its name and descriptions, the managed
 * file formats, and must be able to produce either a VTK reader or a VTK importer.
 * Every reader must be registered to the F3DFactory singleton. This is
 * automatically done by the F3DReaderInstantiator class which is generated
 * by CMake when declaring every reader with the declare_f3d_reader() macro.
 */

#ifndef F3DReader_h
#define F3DReader_h

#include <vtkAlgorithm.h>
#include <vtkImporter.h>
#include <vtkSmartPointer.h>

#include <string>
#include <vector>

class F3DReader
{
public:
  F3DReader() = default;
  virtual ~F3DReader() = default;

  /*
   * Get the name of this reader
   */
  virtual const std::string GetName() const = 0;

  /*
   * Get the short description of this reader
   */
  virtual const std::string GetShortDescription() const = 0;

  /*
   * Get the long description of this reader
   */
  virtual const std::string GetLongDescription() const { return this->GetShortDescription(); }

  /*
   * Get the extensions supported by this reader
   */
  virtual const std::vector<std::string> GetExtensions() const = 0;

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const = 0;

  /*
   * Check if this reader can read the given filename - generally according its extension
   */
  virtual bool CanRead(const std::string& fileName) const;

  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  virtual vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string&) const { return nullptr; }

  /*
   * Apply custom code for the reader
   */
  virtual void ApplyCustomReader(vtkAlgorithm*, const std::string&) const {}

  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  virtual vtkSmartPointer<vtkImporter> CreateSceneReader(const std::string&) const { return nullptr; }

  /*
   * Apply custom code for the importer
   */
  virtual void ApplyCustomImporter(vtkImporter*, const std::string&) const {}
};

#endif
