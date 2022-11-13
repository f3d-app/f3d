/**
 * @class   reader
 * @brief   The basis reader class
 *
 * `reader` is the basic class for every file format readers.
 * It must provide information about its name and descriptions, the managed
 * file formats, and must be able to produce either a VTK reader or a VTK importer.
 * Every reader must be registered to the `factory` singleton. This is
 * automatically done when the plugin is loaded by CMake when declaring every reader
 * with the f3d_declare_reader() macro.
 */

#ifndef f3d_plugin_reader_h
#define f3d_plugin_reader_h

#include <vtkAlgorithm.h>
#include <vtkImporter.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace f3d
{
class reader
{
public:
  reader() = default;
  virtual ~reader() = default;

  /*
   * Get the name of this reader
   */
  virtual const std::string getName() const = 0;

  /*
   * Get the short description of this reader
   */
  virtual const std::string getShortDescription() const = 0;

  /*
   * Get the long description of this reader
   */
  virtual const std::string getLongDescription() const { return this->getShortDescription(); }

  /*
   * Get the extensions supported by this reader
   */
  virtual const std::vector<std::string> getExtensions() const = 0;

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> getMimeTypes() const = 0;

  /*
   * Check if this reader can read the given filename - generally according its extension
   */
  virtual bool canRead(const std::string& fileName) const
  {
    std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    const std::vector<std::string> extensions = this->getExtensions();
    for (auto e : extensions)
    {
      if (e == ext)
      {
        return true;
      }
    }
    return false;
  }

  /*
   * Get the score of this reader.
   * The score is used in case several readers are able to read the file.
   * The reader having the highest score (from 0 to 100) is used to read the file.
   * Default is 50.
   */
  virtual int getScore() const { return 50; }

  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  virtual vtkSmartPointer<vtkAlgorithm> createGeometryReader(const std::string&) const
  {
    return nullptr;
  }

  /*
   * Apply custom code for the reader
   */
  virtual void applyCustomReader(vtkAlgorithm*, const std::string&) const {}

  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  virtual vtkSmartPointer<vtkImporter> createSceneReader(const std::string&) const
  {
    return nullptr;
  }

  /*
   * Apply custom code for the importer
   */
  virtual void applyCustomImporter(vtkImporter*, const std::string&) const {}
};
}

#endif
