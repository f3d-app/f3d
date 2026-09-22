#include <vtkFileResourceStream.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include "vtkF3DArchiveReader.h"

#include <iostream>
#include <string>
#include <vector>

namespace
{
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
}

int TestF3DArchiveReader(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data/";
  bool ret = true;

  vtkNew<vtkF3DArchiveReader> archive;

  // nothing is opened yet
  std::vector<char> content;
  if (archive->Has("3D/3dmodel.model") || archive->Extract("3D/3dmodel.model", content))
  {
    std::cerr << "Unexpected entry found in a closed archive\n";
    ret = false;
  }

  if (archive->Open(nullptr))
  {
    std::cerr << "Unexpectedly opened a null stream\n";
    ret = false;
  }

  vtkNew<NonSeekableStream> nonSeekable;
  if (archive->Open(nonSeekable))
  {
    std::cerr << "Unexpectedly opened a non seekable stream\n";
    ret = false;
  }

  vtkNew<vtkMemoryResourceStream> garbage;
  const std::string notAnArchive = "This is not a zip archive";
  garbage->SetBuffer(notAnArchive.data(), notAnArchive.size());
  if (archive->Open(garbage))
  {
    std::cerr << "Unexpectedly opened an invalid archive\n";
    ret = false;
  }

  // a 3MF file is a zip archive
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open((data + "cube_gears.3mf").c_str()))
  {
    std::cerr << "Cannot open cube_gears.3mf\n";
    return EXIT_FAILURE;
  }

  if (!archive->Open(stream))
  {
    std::cerr << "Cannot open the archive\n";
    return EXIT_FAILURE;
  }
  archive->Print(std::cout);

  if (!archive->Has("[Content_Types].xml") || !archive->Has("3D/3dmodel.model"))
  {
    std::cerr << "Expected entries not found in the archive\n";
    ret = false;
  }

  if (archive->Has("3dmodel.model") || archive->Extract("3dmodel.model", content))
  {
    std::cerr << "Unexpected entry found in the archive\n";
    ret = false;
  }

  if (!archive->Extract("_rels/.rels", content) || content.size() != 259 ||
    std::string(content.data(), 5) != "<?xml")
  {
    std::cerr << "Unexpected content extracted from the archive\n";
    ret = false;
  }

  // a failed open closes the previously opened archive
  if (archive->Open(garbage) || archive->Has("[Content_Types].xml"))
  {
    std::cerr << "The invalid archive was not closed\n";
    ret = false;
  }

  // the same reader can be opened again
  if (!archive->Open(stream) || !archive->Has("[Content_Types].xml"))
  {
    std::cerr << "Cannot reopen the archive\n";
    ret = false;
  }

  archive->Close();
  archive->Close();
  if (archive->Has("[Content_Types].xml"))
  {
    std::cerr << "Unexpected entry found in a closed archive\n";
    ret = false;
  }

  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
