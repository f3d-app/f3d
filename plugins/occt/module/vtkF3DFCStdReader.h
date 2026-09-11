/**
 * @class   vtkF3DFCStdReader
 * @brief   VTK Reader for FreeCAD (.FCStd) files
 *
 * A FCStd file is a zip archive containing a Document.xml file describing the objects
 * of the document, BREP files containing the geometry of these objects, and an optional
 * GuiDocument.xml file describing their visual properties (visibility, colors).
 *
 * This reader lists the objects from Document.xml, reads the BREP geometry of visible
 * objects using OpenCASCADE, resolves App::Link instances and container placements,
 * recovers colors from GuiDocument.xml when present, and outputs a
 * vtkPartitionedDataSetCollection.
 *
 * The quality of the generated mesh is configured using RelativeDeflection,
 * LinearDeflection and AngularDeflection. Reading 1D cells (wires) is optional.
 */

#ifndef vtkF3DFCStdReader_h
#define vtkF3DFCStdReader_h

#include <vtkPartitionedDataSetCollectionAlgorithm.h>
#include <vtkSmartPointer.h>

#include <memory>

class vtkResourceStream;
class vtkF3DFCStdReader : public vtkPartitionedDataSetCollectionAlgorithm
{
public:
  static vtkF3DFCStdReader* New();
  vtkTypeMacro(vtkF3DFCStdReader, vtkPartitionedDataSetCollectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Set/Get the linear deflection.
   * This value limits the distance between a curve and the resulting tessellation.
   * Default is 0.1
   */
  vtkGetMacro(LinearDeflection, double);
  vtkSetMacro(LinearDeflection, double);
  ///@}

  ///@{
  /**
   * Set/Get the angular deflection.
   * This value limits the angle between two subsequent segments.
   * Default is 0.5
   */
  vtkGetMacro(AngularDeflection, double);
  vtkSetMacro(AngularDeflection, double);
  ///@}

  ///@{
  /**
   * Set/Get relative deflection.
   * Determine if the deflection values are relative to object size.
   * Default is false
   */
  vtkGetMacro(RelativeDeflection, bool);
  vtkSetMacro(RelativeDeflection, bool);
  vtkBooleanMacro(RelativeDeflection, bool);
  ///@}

  ///@{
  /**
   * Enable/Disable 1D cells read. If enabled, surface boundaries are read.
   * Default is false
   */
  vtkGetMacro(ReadWire, bool);
  vtkSetMacro(ReadWire, bool);
  vtkBooleanMacro(ReadWire, bool);
  ///@}

  ///@{
  /**
   * Specify stream to read from
   * When both `Stream` and `Filename` are set, stream is used.
   */
  void SetStream(vtkResourceStream* stream);
  vtkResourceStream* GetStream();
  ///@}

  ///@{
  /**
   * Get/Set the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

  /**
   * Overridden to take into account mtime from the internal vtkResourceStream.
   */
  vtkMTimeType GetMTime() override;

  /**
   * Return true if, after a quick check, it looks like the provided stream is a
   * FCStd file: a zip archive containing a Document.xml entry.
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DFCStdReader();
  ~vtkF3DFCStdReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DFCStdReader(const vtkF3DFCStdReader&) = delete;
  void operator=(const vtkF3DFCStdReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;

  std::string FileName;
  vtkSmartPointer<vtkResourceStream> Stream;
  double LinearDeflection = 0.1;
  double AngularDeflection = 0.5;
  bool RelativeDeflection = false;
  bool ReadWire = false;
};

#endif
