#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkExecutive.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkResourceStream.h>

#include "vtkF3DFCStdReader.h"

#include <iostream>
#include <string>
#include <vector>

namespace
{
class ErrorEventCallback : public vtkCommand
{
public:
  static ErrorEventCallback* New()
  {
    return new ErrorEventCallback;
  }

  void Execute(vtkObject*, unsigned long, void* data) override
  {
    this->Messages.emplace_back(static_cast<const char*>(data));
  }

  std::vector<std::string> Messages;
};

// a stream that can be read but not seeked, which cannot contain an archive
class NonSeekableStream : public vtkResourceStream
{
public:
  static NonSeekableStream* New();
  vtkTypeMacro(NonSeekableStream, vtkResourceStream);

  std::size_t Read(void*, std::size_t) override
  {
    return 0;
  }

  bool EndOfStream() override
  {
    return true;
  }

protected:
  NonSeekableStream()
    : vtkResourceStream(false)
  {
  }
};
vtkStandardNewMacro(NonSeekableStream);

bool TestError(const std::string& filename, const std::string& expectedError)
{
  vtkNew<ErrorEventCallback> errorCallback;
  vtkNew<vtkCallbackCommand> silentCallback;
  vtkNew<vtkF3DFCStdReader> reader;
  reader->AddObserver(vtkCommand::ErrorEvent, errorCallback);
  reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, silentCallback);
  reader->SetFileName(filename);
  reader->Update();

  if (errorCallback->Messages.empty() ||
    errorCallback->Messages.front().find(expectedError) == std::string::npos)
  {
    std::cerr << "Expected error \"" << expectedError << "\" for " << filename << "\n";
    return false;
  }
  if (reader->GetOutput()->GetNumberOfPartitionedDataSets() != 0)
  {
    std::cerr << "Unexpected output for " << filename << "\n";
    return false;
  }
  return true;
}
}

int TestF3DFCStdReaderErrors(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  bool ret = true;

  ret &= TestError("", "Cannot open file");
  ret &= TestError(data + "/f3d.stp", "Cannot open FCStd archive");
  ret &= TestError(data + "/malformed_no_objects.FCStd", "Cannot parse Document.xml");
  ret &= TestError(data + "/malformed_no_objectdata.FCStd", "Cannot parse Document.xml");

  vtkNew<NonSeekableStream> nonSeekable;
  if (vtkF3DFCStdReader::CanReadFile(nullptr) || vtkF3DFCStdReader::CanReadFile(nonSeekable))
  {
    std::cerr << "Unexpectedly accepted a null or non seekable stream\n";
    ret = false;
  }

  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
