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
std::vector<uint8_t> BuildHeader(
  uint32_t version, uint32_t jsonBlockSize, uint32_t secondarySize, uint32_t jsonLen)
{
  std::vector<uint8_t> header(16, 0);
  std::memcpy(header.data(), &version, sizeof(version));
  std::memcpy(header.data() + 4, &jsonBlockSize, sizeof(jsonBlockSize));
  std::memcpy(header.data() + 8, &secondarySize, sizeof(secondarySize));
  std::memcpy(header.data() + 12, &jsonLen, sizeof(jsonLen));
  return header;
}

std::vector<uint8_t> BuildMeshHeader(
  uint32_t indexCount, uint32_t coordCount, uint32_t reserved = 0)
{
  std::vector<uint8_t> header(12, 0);
  std::memcpy(header.data(), &indexCount, sizeof(indexCount));
  std::memcpy(header.data() + 4, &coordCount, sizeof(coordCount));
  std::memcpy(header.data() + 8, &reserved, sizeof(reserved));
  return header;
}

std::vector<uint8_t> BuildLysData(const std::string& jsonStr,
  const std::vector<uint8_t>& geomBlob = {}, uint32_t jsonBlockSizeOverride = 0)
{
  const uint32_t jsonLen = static_cast<uint32_t>(jsonStr.size());
  const uint32_t jsonBlockSize = jsonBlockSizeOverride > 0 ? jsonBlockSizeOverride : jsonLen;
  const auto header = BuildHeader(4, jsonBlockSize, 0, jsonLen);

  std::vector<uint8_t> data(16 + jsonBlockSize + geomBlob.size(), 0);
  std::memcpy(data.data(), header.data(), 16);
  if (!jsonStr.empty())
  {
    std::memcpy(data.data() + 16, jsonStr.data(), jsonStr.size());
  }
  if (!geomBlob.empty())
  {
    std::memcpy(data.data() + 16 + jsonBlockSize, geomBlob.data(), geomBlob.size());
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
  const vtkNew<vtkStringOutputWindow> outputWindow;
  vtkOutputWindow::SetInstance(outputWindow);

  if (vtkF3DLYSReader::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  const std::string path = std::string(argv[1]) + "data/bunny.lys";

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
      std::cerr << "Unexpected CanReadFile failure on bunny.lys (stream)\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DLYSReader> reader;
    reader->SetStream(stream);
    reader->Update();

    const vtkIdType nPoints = reader->GetOutput()->GetNumberOfPoints();
    if (nPoints != 1887)
    {
      std::cerr << "Incorrect number of points: " << nPoints << "\n";
      return EXIT_FAILURE;
    }

    const vtkIdType nPolys = reader->GetOutput()->GetNumberOfPolys();
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

    const vtkIdType nPoints = reader->GetOutput()->GetNumberOfPoints();
    if (nPoints != 1887)
    {
      std::cerr << "Incorrect number of points: " << nPoints << "\n";
      return EXIT_FAILURE;
    }

    const vtkIdType nPolys = reader->GetOutput()->GetNumberOfPolys();
    if (nPolys != 3851)
    {
      std::cerr << "Incorrect number of polygons: " << nPolys << "\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with non-LYS file
  {
    const std::string pathVtp = std::string(argv[1]) + "data/f3d.vtp";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathVtp.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on f3d.vtp\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with empty file
  {
    const std::string pathEmpty = std::string(argv[1]) + "data/empty.splat";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathEmpty.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on empty file\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with invalid/corrupted file
  {
    const std::string pathInvalid = std::string(argv[1]) + "data/invalid.splat";
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

  // 2. Invalid container header dimensions
  {
    const auto header = BuildHeader(4, 1, 0, 1);
    if (!TestReaderUpdateFails(header))
    {
      std::cerr << "Unexpected success on invalid container header dimensions\n";
      return EXIT_FAILURE;
    }
  }

  // 3. Truncated JSON manifest
  {
    const auto header = BuildHeader(4, 100, 0, 100);
    if (!TestReaderUpdateFails(header))
    {
      std::cerr << "Unexpected success on truncated JSON manifest\n";
      return EXIT_FAILURE;
    }
  }

  // 4. Malformed JSON manifest
  {
    const auto data = BuildLysData("not a json");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on malformed JSON\n";
      return EXIT_FAILURE;
    }
  }

  // 5. Missing mangoFiles key
  {
    const auto data = BuildLysData(R"({"version": 1})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on missing mangoFiles\n";
      return EXIT_FAILURE;
    }
  }

  // 6. Missing geometry .bin entry (only scene.bin, non-.bin entries, or exact ".bin" without
  // prefix)
  {
    const auto data = BuildLysData(
      R"({"mangoFiles": {".bin": {"offset": "0", "size": 0}, "scene.bin": {"offset": "0", "size": 0}, "other.txt": {"offset": "0", "size": 0}}})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on missing geometry bin\n";
      return EXIT_FAILURE;
    }
  }

  // 7. Invalid metadata in geometry entry (non-numeric offset string or invalid size)
  {
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "not_a_number", "size": 100}}})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on non-numeric geometry offset\n";
      return EXIT_FAILURE;
    }
  }
  {
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": "not_an_int"}}})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on invalid geometry size\n";
      return EXIT_FAILURE;
    }
  }

  // 8. Geometry blob read fails (stream truncated before geomSize)
  {
    const auto data = BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 100}}})");
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on truncated geometry blob\n";
      return EXIT_FAILURE;
    }
  }

  // 9. Geometry blob too small for mesh header (< 12 bytes)
  {
    const std::vector<uint8_t> smallGeom(8, 0);
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 8}}})", smallGeom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on geometry blob smaller than mesh header\n";
      return EXIT_FAILURE;
    }
  }

  // 10. Mesh header indexCount not a multiple of 3
  {
    const auto geom = BuildMeshHeader(4, 0, 0);
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 12}}})", geom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on indexCount not multiple of 3\n";
      return EXIT_FAILURE;
    }
  }

  // 11. Mesh header coordCount not a multiple of 3
  {
    const auto geom = BuildMeshHeader(3, 4, 0);
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 12}}})", geom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on coordCount not multiple of 3\n";
      return EXIT_FAILURE;
    }
  }

  // 12. Geometry blob claims more data than available
  {
    const auto geom = BuildMeshHeader(3, 3, 0);
    const auto data =
      BuildLysData(R"({"mangoFiles": {"mesh.bin": {"offset": "0", "size": 12}}})", geom);
    if (!TestReaderUpdateFails(data))
    {
      std::cerr << "Unexpected success on truncated mesh data\n";
      return EXIT_FAILURE;
    }
  }

  // 13. Out-of-bounds triangle vertex indices
  {
    std::vector<uint8_t> geom(12 + 3 * sizeof(uint32_t) + 3 * sizeof(float));
    const auto mhdr = BuildMeshHeader(3, 3, 0); // 1 point
    std::memcpy(geom.data(), mhdr.data(), 12);
    const uint32_t indices[3] = { 0, 5, 0 }; // 5 >= 1 is out of bounds
    std::memcpy(geom.data() + 12, indices, sizeof(indices));
    const float coords[3] = { 0.0f, 0.0f, 0.0f };
    std::memcpy(geom.data() + 12 + sizeof(indices), coords, sizeof(coords));
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
    const auto header = BuildHeader(4, 1, 0, 1);
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(header.data(), header.size(), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with jsonBlockSize < 2\n";
      return EXIT_FAILURE;
    }
  }
  {
    // jsonLen > jsonBlockSize
    const auto header = BuildHeader(4, 10, 0, 20);
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(header.data(), header.size(), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with jsonLen > jsonBlockSize\n";
      return EXIT_FAILURE;
    }
  }
  {
    // Stream truncated before jsonLen bytes in CanReadFile
    const auto header = BuildHeader(4, 20, 0, 10);
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(header.data(), header.size(), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with truncated JSON stream\n";
      return EXIT_FAILURE;
    }
  }
  {
    // jsonLen < 2 with jsonBlockSize >= 2
    const auto header = BuildHeader(4, 10, 0, 1);
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(header.data(), header.size(), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with jsonLen < 2\n";
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
  {
    // Valid JSON but missing "mangoFiles" key in CanReadFile
    const auto data = BuildLysData(R"({"version": 1})");
    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(data.data(), data.size(), true);
    if (vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success with valid JSON missing mangoFiles\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
