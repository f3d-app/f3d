/**
 * @class   vtkF3DWebIFCReader
 * @brief   VTK Reader for IFC (Industry Foundation Classes) files
 *
 * This reader is using the web-ifc library to parse and generate geometry
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

#ifndef vtkF3DWebIFCReader_h
#define vtkF3DWebIFCReader_h

#include <vtkPolyDataAlgorithm.h>
#include <vtkResourceStream.h>
#include <vtkSmartPointer.h>

#include <memory>

class vtkF3DWebIFCReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DWebIFCReader* New();
  vtkTypeMacro(vtkF3DWebIFCReader, vtkPolyDataAlgorithm) void PrintSelf(
    ostream& os, vtkIndent indent) override;

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
  vtkSetSmartPointerMacro(Stream, vtkResourceStream);
  vtkGetSmartPointerMacro(Stream, vtkResourceStream);
  ///@}

  ///@{
  /**
   * Set/Get the number of segments used for circular geometry.
   * Higher values produce smoother curves but more geometry.
   * Default is 12.
   */
  vtkGetMacro(CircleSegments, uint16_t);
  vtkSetMacro(CircleSegments, uint16_t);
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
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read.
   *
   * This only checks the first 32 lines for an "ISO-10303-21;" header
   * and a FILE_SCHEMA containing an IFC schema identifier (e.g. 'IFC2X3', 'IFC4').
   */
  static bool CanReadFile(vtkResourceStream* stream);

  /**
   * Overridden to take into account mtime from the internal vtkResourceStream.
   */
  vtkMTimeType GetMTime() override;

protected:
  vtkF3DWebIFCReader();
  ~vtkF3DWebIFCReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DWebIFCReader(const vtkF3DWebIFCReader&) = delete;
  void operator=(const vtkF3DWebIFCReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;

  std::string FileName;
  vtkSmartPointer<vtkResourceStream> Stream;

  uint16_t CircleSegments = 12;
  bool ReadOpenings = false;
  bool ReadSpaces = false;
};

#endif
