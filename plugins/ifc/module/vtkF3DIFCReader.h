/**
 * @class   vtkF3DIFCReader
 * @brief   VTK Reader for IFC (Industry Foundation Classes) files
 *
 * This reader is based on webifc library to parse and generate geometry
 * from IFC files used in BIM (Building Information Modeling).
 *
 * The reader supports IFC2x3, IFC4, and IFC4x3 schemas.
 *
 * Supported features:
 * - Geometry (vertices, normals, triangles)
 * - Colors (RGBA with transparency from IfcSurfaceStyleRendering)
 *
 * Not supported by webifc:
 * - Full materials (specular, roughness, reflectance method)
 * - Textures
 */

#ifndef vtkF3DIFCReader_h
#define vtkF3DIFCReader_h

#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

#include <memory>

class vtkResourceStream;

class vtkF3DIFCReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DIFCReader* New();
  vtkTypeMacro(vtkF3DIFCReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Get/Set the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

  ///@{
  /**
   * Specify stream to read from.
   * When both Stream and FileName are set, stream is used.
   */
  void SetStream(vtkResourceStream* stream);
  vtkResourceStream* GetStream();
  ///@}

  ///@{
  /**
   * Set/Get the number of segments used for circular geometry.
   * Higher values produce smoother curves but more geometry.
   * Default is 12.
   */
  vtkGetMacro(CircleSegments, int);
  vtkSetMacro(CircleSegments, int);
  ///@}

  ///@{
  /**
   * Enable/Disable reading of IfcOpeningElement entities.
   * Default is false.
   */
  vtkGetMacro(ReadOpenings, bool);
  vtkSetMacro(ReadOpenings, bool);
  vtkBooleanMacro(ReadOpenings, bool);
  ///@}

  ///@{
  /**
   * Enable/Disable reading of IfcSpace entities.
   * Default is false.
   */
  vtkGetMacro(ReadSpaces, bool);
  vtkSetMacro(ReadSpaces, bool);
  vtkBooleanMacro(ReadSpaces, bool);
  ///@}

  /**
   * Check if the stream can be read by this reader.
   */
  static bool CanReadFile(vtkResourceStream* stream);

  /**
   * Overridden to take into account mtime from the internal vtkResourceStream.
   */
  vtkMTimeType GetMTime() override;

protected:
  vtkF3DIFCReader();
  ~vtkF3DIFCReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DIFCReader(const vtkF3DIFCReader&) = delete;
  void operator=(const vtkF3DIFCReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;

  std::string FileName;
  vtkSmartPointer<vtkResourceStream> Stream;

  int CircleSegments = 12;
  bool ReadOpenings = false;
  bool ReadSpaces = false;
};

#endif
