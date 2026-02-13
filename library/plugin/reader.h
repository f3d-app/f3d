#ifndef f3d_reader_h
#define f3d_reader_h

#include <vtkAlgorithm.h>
#include <vtkFileResourceStream.h>
#include <vtkImporter.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <string>
#include <vector>

class vtkResourceStream;
namespace f3d
{
/**
 * @class   reader
 * @brief   The basis reader class
 *
 * `reader` is the basic class for every file format readers.
 * It must provide information about its name and descriptions, the managed
 * file formats, and must be able to produce either a VTK reader or a VTK importer.
 * Every reader must be registered to the `factory` singleton. This is
 * automatically done when the plugin is loaded by CMake when declaring every reader
 * with the f3d_plugin_declare_reader() macro.
 *
 * @warning This file is used internally by the plugin SDK, it is not intended to be included
 * directly by libf3d users.
 */
class reader
{
public:
  reader() = default;
  virtual ~reader() = default;

  /**
   * Get the name of this reader
   */
  virtual const std::string getName() const = 0;

  /**
   * Get the short description of this reader
   */
  virtual const std::string getShortDescription() const = 0;

  /**
   * Get the long description of this reader
   */
  virtual const std::string getLongDescription() const
  {
    return this->getShortDescription();
  }

  /**
   * Get the extensions supported by this reader
   */
  virtual const std::vector<std::string> getExtensions() const = 0;

  /**
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> getMimeTypes() const = 0;

  /**
   * Check if this reader can read the given filename - according to its extension and header check
   */
  virtual bool canRead(const std::string& fileName) const
  {
    std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    const std::vector<std::string>& extensions = this->getExtensions();

    if (!std::any_of(
          extensions.begin(), extensions.end(), [&](const std::string& s) { return s == ext; }))
    {
      return false;
    }

    vtkNew<vtkFileResourceStream> stream;
    if (!stream->Open(fileName.c_str()))
    {
      return false;
    }

    return this->canRead(stream);
  }

  /**
   * Return true if this reader could be able to read provided stream,
   * false if it is sure it cannot.
   * Default implementation returns true.
   */
  virtual bool canRead(vtkResourceStream*) const
  {
    return true;
  }

  /**
   * Get the score of this reader.
   * The score is used in case several readers are able to read the file.
   * The reader having the highest score (from 0 to 100) is used to read the file.
   * Default is 50.
   */
  virtual int getScore() const
  {
    return 50;
  }

  /**
   * Return true if this reader can create a geometry reader
   * false otherwise
   */
  virtual bool hasGeometryReader()
  {
    return false;
  }

  /**
   * Create the geometry reader (VTK reader) for the given filename
   */
  virtual vtkSmartPointer<vtkAlgorithm> createGeometryReader(const std::string&) const
  {
    return nullptr;
  }

  /**
   * Create the geometry reader (VTK reader) for the given stream
   */
  virtual vtkSmartPointer<vtkAlgorithm> createGeometryReader(vtkResourceStream*) const
  {
    return nullptr;
  }

  /**
   * Apply custom code for the reader
   */
  virtual void applyCustomReader(vtkAlgorithm*, const std::string&, vtkResourceStream*) const
  {
  }

  /**
   * Return true if this reader can create a scene reader
   * false otherwise
   */
  virtual bool hasSceneReader()
  {
    return false;
  }

  /**
   * Create the scene reader (VTK importer) for the given filename
   */
  virtual vtkSmartPointer<vtkImporter> createSceneReader(const std::string&) const
  {
    return nullptr;
  }

  /**
   * Create the scene reader (VTK importer) for the given stream
   */
  virtual vtkSmartPointer<vtkImporter> createSceneReader(vtkResourceStream*) const
  {
    return nullptr;
  }

  /**
   * Apply custom code for the importer
   */
  virtual void applyCustomImporter(vtkImporter*, const std::string&, vtkResourceStream*) const
  {
  }

  /**
   * Return true if this reader supports stream
   * false otherwise
   */
  virtual bool supportsStream() const
  {
    return false;
  }

  /**
   * Set a reader option
   * Return true if the option was found (and set), false otherwise
   */
  bool setReaderOption(const std::string& name, const std::string& value)
  {
    auto iter = this->ReaderOptions.find(name);
    if (iter == this->ReaderOptions.end())
    {
      return false;
    }
    iter->second = value;
    return true;
  }

  /**
   * Return the list of all reader option names
   */
  std::vector<std::string> getAllReaderOptionNames() const
  {
    std::vector<std::string> keys;
    keys.reserve(this->ReaderOptions.size());
    for (const auto& [key, value] : this->ReaderOptions)
    {
      keys.push_back(key);
    }
    return keys;
  }

protected:
  std::map<std::string, std::string> ReaderOptions;
};
}

#endif
