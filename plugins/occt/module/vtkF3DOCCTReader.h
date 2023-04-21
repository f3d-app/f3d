/**
 * @class   vtkF3DOCCTReader
 * @brief   VTK Reader for STEP and IGES files using OpenCASCADE
 *
 * This reader is based on OpenCASCADE and use XCAF toolkits (TKXDESTEP and TKXDEIGES)
 * if available to read the names and the colors. If not available, TKSTEP and TKIGES are
 * used but no names or colors are read.
 * The quality of the generated mesh is configured using RelativeDeflection, LinearDeflection,
 * and LinearDeflection.
 * Reading 1D cells (wires) is optional.
 *
 */

#ifndef vtkF3DOCCTReader_h
#define vtkF3DOCCTReader_h

#include <vtkMultiBlockDataSetAlgorithm.h>
#include <vtkVersion.h>

#include <memory>

class vtkInformationDoubleVectorKey;

class vtkF3DOCCTReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkF3DOCCTReader* New();
  vtkTypeMacro(vtkF3DOCCTReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum FILE_FORMAT : unsigned char
  {
    STEP,
    IGES
  };

  ///@{
  /**
   * Set the file format to read.
   * It can be either STEP or IGES.
   * Default is FILE_FORMAT::STEP
   */
  vtkSetMacro(FileFormat, FILE_FORMAT);
  ///@}

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
   * Get/Set the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

protected:
  vtkF3DOCCTReader();
  ~vtkF3DOCCTReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DOCCTReader(const vtkF3DOCCTReader&) = delete;
  void operator=(const vtkF3DOCCTReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;

  std::string FileName;

  double LinearDeflection = 0.1;
  double AngularDeflection = 0.5;
  bool RelativeDeflection = false;
  bool ReadWire = false;
  FILE_FORMAT FileFormat = FILE_FORMAT::STEP;
};

#endif
