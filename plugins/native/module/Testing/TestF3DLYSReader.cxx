#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkFileResourceStream.h>
#include <vtkLogger.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkStringOutputWindow.h>

#include "vtkF3DLYSReader.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace
{
struct ContainerHeader
{
  uint32_t version = 4;
  uint32_t jsonBlockSize = 0;
  uint32_t secondarySize = 0;
  uint32_t jsonLen = 0;
};

struct MeshHeader
{
  uint32_t indexCount = 0;
  uint32_t coordCount = 0;
  uint32_t reserved = 0;
};

std::vector<uint8_t> BuildLysData(const std::string& jsonStr,
  const std::vector<uint8_t>& geomBlob = {}, uint32_t jsonBlockSizeOverride = 0)
{
  ContainerHeader header;
  header.version = 4;
  header.jsonLen = static_cast<uint32_t>(jsonStr.size());
  header.jsonBlockSize = jsonBlockSizeOverride > 0 ? jsonBlockSizeOverride : header.jsonLen;
  header.secondarySize = 0;

  std::vector<uint8_t> data(sizeof(ContainerHeader) + header.jsonBlockSize + geomBlob.size(), 0);
  std::memcpy(data.data(), &header, sizeof(ContainerHeader));
  if (!jsonStr.empty())
  {
    std::memcpy(data.data() + sizeof(ContainerHeader), jsonStr.data(), jsonStr.size());
  }
  if (!geomBlob.empty())
  {
    std::memcpy(data.data() + sizeof(ContainerHeader) + header.jsonBlockSize, geomBlob.data(),
      geomBlob.size());
  }

  return data;
}

bool TestReaderUpdateFails(const std::vector<uint8_t>& data)
{
  bool errorEmitted = false;
  vtkNew<vtkCallbackCommand> nullCallback;
  nullCallback->SetClientData(&errorEmitted);
  nullCallback->SetCallback([](vtkObject*, unsigned long, void* clientData, void*)
    { *static_cast<bool*>(clientData) = true; });

  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(data.data(), data.size(), true);

  vtkNew<vtkF3DLYSReader> reader;
  reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
  reader->SetStream(stream);
  reader->Update();

  return errorEmitted && reader->GetOutput()->GetNumberOfPolys() == 0;
}
}

int TestF3DLYSReader(int vtkNotUsed(argc), char* argv[])
{
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
  vtkNew<vtkStringOutputWindow> outputWindow;
  vtkOutputWindow::SetInstance(outputWindow);

  if (vtkF3DLYSReader::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  std::string path = std::string(argv[1]) + "data/bunny.lys";

  // Test reading from stream
  {
    vtkNew<vtkFileResourceStream> stream;
    if (!stream->Open(path.c_str()))
    {
      std::cerr << "Cannot open file: " << path << "\n";
      return EXIT_FAILURE;
    }

    if (!vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile failure on bunny.lys\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DLYSReader> reader;
    reader->SetStream(stream);
    reader->Update();

    vtkIdType nPoints = reader->GetOutput()->GetNumberOfPoints();
    if (nPoints != 1887)
    {
      std::cerr << "Incorrect number of points: " << nPoints << "\n";
      return EXIT_FAILURE;
    }

    vtkIdType nPolys = reader->GetOutput()->GetNumberOfPolys();
    if (nPolys != 3851)
    {
      std::cerr << "Incorrect number of polygons: " << nPolys << "\n";
      return EXIT_FAILURE;
    }
  }

  // Test reading from file name
  {
    vtkNew<vtkF3DLYSReader> reader;
    reader->SetFileName(path.c_str());
    reader->Update();

    vtkIdType nPoints = reader->GetOutput()->GetNumberOfPoints();
    if (nPoints != 1887)
    {
      std::cerr << "Incorrect number of points with SetFileName: " << nPoints << "\n";
      return EXIT_FAILURE;
    }

    vtkIdType nPolys = reader->GetOutput()->GetNumberOfPolys();
    if (nPolys != 3851)
    {
      std::cerr << "Incorrect number of polygons with SetFileName: " << nPolys << "\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with non-LYS valid file
  {
    std::string pathVtp = std::string(argv[1]) + "data/f3d.vtp";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathVtp.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on f3d.vtp\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with empty file
  {
    std::string pathEmpty = std::string(argv[1]) + "data/empty.splat";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathEmpty.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on empty file\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with invalid/corrupted file
  {
    std::string pathInvalid = std::string(argv[1]) + "data/invalid.splat";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathInvalid.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on invalid file\n";
      return EXIT_FAILURE;
    }
  }

  // Test reading from non-existent file name
  {
    vtkNew<vtkCallbackCommand> nullCallback;
    nullCallback->SetCallback([](vtkObject*, unsigned long, void*, void*) {});

    vtkNew<vtkF3DLYSReader> reader;
    reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->SetFileName("non_existent_file_for_coverage.lys");
    reader->Update();

    if (reader->GetOutput()->GetNumberOfPoints() != 0)
    {
      std::cerr << "Unexpected points with non-existent file\n";
      return EXIT_FAILURE;
    }
  }

  // Test RequestData error branches
  // 1. Short container header
  {
    const std::vector<uint8_t> shortData = { 1, 2, 3, 4 };
    if (!TestReaderUpdateFails(shortData))
    {
      std::cerr << "Unexpected success on short container header\n";
      return EXIT_FAILURE;
    }
  }

  // 2. Truncated JSON manifest
  {
    ContainerHeader header;
    header.version = 4;
    header.jsonLen = 100;
    header.jsonBlockSize = 100;
    header.secondarySize = 0;
    std::vector<uint8_t> data(sizeof(ContainerHeader));
    std::memcpy(data.data(), &header, sizeof(ContainerHeader));
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on truncated JSON manifest\n";
      return EXIT_FAILURE;
    }
  }

  // 3. Malformed JSON manifest
  {
    const auto data = BuildLysData("not a json");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on malformed JSON\n";
      return EXIT_FAILURE;
    }
  }

  // 4. Missing mangoFiles key
  {
    const auto data = BuildLysData(R"({"version": 1})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on missing mangoFiles\n";
      return EXIT_FAILURE;
    }
  }

  // 5. Missing geometry .bin entry (only scene.bin or non-.bin entries)
  {
    const auto data = BuildLysData(
      R"({"mangoFiles": {"scene.bin": {"offset": "0", "size": 0}, "other.txt": {"offset": "0", "size": 0}}})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on missing geometry bin\n";
      return EXIT_FAILURE;
    }
  }

  // 6. Geometry blob read fails (stream truncated before geomSize)
  {
    const auto data = BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 100}}})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on truncated geometry blob\n";
      return EXIT_FAILURE;
    }
  }

  // 7. Geometry blob too small for MeshHeader (< 12 bytes)
  {
    const std::vector<uint8_t> smallGeom(8, 0);
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 8}}})", smallGeom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on geometry blob smaller than MeshHeader\n";
      return EXIT_FAILURE;
    }
  }

  // 8. MeshHeader indexCount not a multiple of 3
  {
    MeshHeader meshHeader;
    meshHeader.indexCount = 4;
    meshHeader.coordCount = 0;
    meshHeader.reserved = 0;
    std::vector<uint8_t> geom(sizeof(MeshHeader));
    std::memcpy(geom.data(), &meshHeader, sizeof(MeshHeader));
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 12}}})", geom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on indexCount not multiple of 3\n";
      return EXIT_FAILURE;
    }
  }

  // 9. Geometry blob claims more data than available
  {
    MeshHeader meshHeader;
    meshHeader.indexCount = 3;
    meshHeader.coordCount = 3;
    meshHeader.reserved = 0;
    std::vector<uint8_t> geom(sizeof(MeshHeader));
    std::memcpy(geom.data(), &meshHeader, sizeof(MeshHeader));
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 12}}})", geom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on truncated mesh data\n";
      return EXIT_FAILURE;
    }
  }

  // 10. Out-of-bounds triangle vertex indices
  {
    MeshHeader meshHeader;
    meshHeader.indexCount = 3;
    meshHeader.coordCount = 3; // 1 point
    meshHeader.reserved = 0;
    std::vector<uint8_t> geom(sizeof(MeshHeader) + 3 * sizeof(uint32_t) + 3 * sizeof(float));
    std::memcpy(geom.data(), &meshHeader, sizeof(MeshHeader));
    const uint32_t indices[3] = { 0, 5, 0 }; // 5 >= 1 is out of bounds
    std::memcpy(geom.data() + sizeof(MeshHeader), indices, sizeof(indices));
    const float coords[3] = { 0.0f, 0.0f, 0.0f };
    std::memcpy(geom.data() + sizeof(MeshHeader) + sizeof(indices), coords, sizeof(coords));
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 36}}})", geom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on out-of-bounds vertex index\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile invalid branches
  {
    // jsonBlockSize < 2
    ContainerHeader header;
    header.version = 4;
    header.jsonBlockSize = 1;
    header.jsonLen = 1;
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(&header, sizeof(header), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with jsonBlockSize < 2\n";
      return EXIT_FAILURE;
    }
  }
  {
    // jsonLen > jsonBlockSize
    ContainerHeader header;
    header.version = 4;
    header.jsonBlockSize = 10;
    header.jsonLen = 20;
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(&header, sizeof(header), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with jsonLen > jsonBlockSize\n";
      return EXIT_FAILURE;
    }
  }
  {
    // Stream truncated before jsonLen bytes in CanReadFile
    ContainerHeader header;
    header.version = 4;
    header.jsonBlockSize = 20;
    header.jsonLen = 10;
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(&header, sizeof(header), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with truncated JSON stream\n";
      return EXIT_FAILURE;
    }
  }
  {
    // Invalid JSON in CanReadFile
    const auto data = BuildLysData("not json");
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(data.data(), data.size(), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with bad JSON\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
