/**
 * @class   vtkF3DMetaReader
 * @brief   A meta reader that can read multiple type of files
 *
 */

#ifndef vtkF3DMetaReader_h
#define vtkF3DMetaReader_h

#include <vtkDataObjectAlgorithm.h>
#include <vtkSmartPointer.h>

class F3DReader;

class vtkF3DMetaReader : public vtkDataObjectAlgorithm
{
public:
  static vtkF3DMetaReader* New();
  vtkTypeMacro(vtkF3DMetaReader, vtkDataObjectAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * Set/Get the name of the file from which to read.
   */
  void SetFileName(const std::string& fileName);
  vtkGetStringMacro(FileName);
  //@}

  /**
   * Returns true if the internal reader is correctly initialized.
   */
  bool IsReaderValid() { return this->InternalReader != nullptr; }

  /**
   * Transfer to the InternalReader
   */
  int ProcessRequest(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  /**
   * Transfer to the InternalReader
   */
  vtkInformation* GetOutputInformation(int port);

  /**
   * Get the F3D reader (set once filename has been provided).
   */
  F3DReader* GetReader() { return this->Reader; }

protected:
  vtkF3DMetaReader();
  ~vtkF3DMetaReader() override;

  /**
   * Protected method used internally to set the filename
   */
  vtkSetStringMacro(FileName);

  /**
   * Transfer to the InternalReader
   */
  int FillOutputPortInformation(int, vtkInformation*) override;

private:
  vtkF3DMetaReader(const vtkF3DMetaReader&) = delete;
  void operator=(const vtkF3DMetaReader&) = delete;

  vtkSmartPointer<vtkAlgorithm> InternalReader;
  F3DReader* Reader = nullptr;
  char* FileName = nullptr;
};

#endif
