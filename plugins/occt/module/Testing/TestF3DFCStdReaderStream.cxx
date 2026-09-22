#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPartitionedDataSetCollection.h>

#include "vtkF3DFCStdReader.h"

#include <iostream>

int TestF3DFCStdReaderStream(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  const std::string filename = data + "/colored_visibility.FCStd";

  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file " << filename << "\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DFCStdReader> reader;
  reader->SetStream(stream);
  reader->SetStream(stream);
  if (reader->GetStream() != stream)
  {
    std::cerr << "Unexpected stream\n";
    return EXIT_FAILURE;
  }
  if (reader->GetMTime() < stream->GetMTime())
  {
    std::cerr << "The reader modification time does not follow the stream one\n";
    return EXIT_FAILURE;
  }

  // the file name is ignored when a stream is set
  reader->SetFileName(data + "/missing.FCStd");
  if (reader->GetFileName() != data + "/missing.FCStd")
  {
    std::cerr << "Unexpected file name\n";
    return EXIT_FAILURE;
  }
  reader->RelativeDeflectionOn();
  reader->Update();

  if (reader->GetOutput()->GetNumberOfPartitionedDataSets() != 1)
  {
    std::cerr << "Expected 1 partition from the stream, got "
              << reader->GetOutput()->GetNumberOfPartitionedDataSets() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
