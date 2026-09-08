#include "vtkF3DLYSReader.h"

#include "vtk_nlohmannjson.h"
#include <vtkCellArray.h>
#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkVersion.h>
#include VTK_NLOHMANN_JSON(json.hpp)

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DLYSReader);

//----------------------------------------------------------------------------
vtkF3DLYSReader::vtkF3DLYSReader()
{
  this->SetNumberOfInputPorts(0);
}
//----------------------------------------------------------------------------
namespace
{
struct ContainerHeader
{
  uint32_t version;
  uint32_t jsonBlockSize;
  uint32_t secondarySize;
  uint32_t jsonLen;
};

struct MeshHeader
{
  uint32_t indexCount;
  uint32_t coordCount;
  uint32_t reserved;
};
}
//----------------------------------------------------------------------------
int vtkF3DLYSReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  // 1. Open stream
  vtkSmartPointer<vtkResourceStream> stream;

#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 4, 20250501)
  if (this->Stream)
  {
    stream = this->Stream;
  }
  else
#endif
  {
    vtkNew<vtkFileResourceStream> fileStream;
    fileStream->Open(this->FileName);
    stream = fileStream;
  }
  // Seek to the beginning in case the stream was already read (e.g. by CanReadFile).
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  // Read the 16-byte container header
  ContainerHeader header;
  if (stream->Read(&header, sizeof(ContainerHeader)) != sizeof(ContainerHeader))
  {
    vtkErrorMacro("Failed to read LYS container header");
    return 0;
  }
  const uint32_t jsonBlockSize = header.jsonBlockSize; // padded size used to seek past the block
  const uint32_t jsonLen = header.jsonLen;             // actual JSON content length
  // Read and parse the JSON manifest (jsonLen bytes, padded to jsonBlockSize in the file)
  std::vector<char> jsonBuf(jsonLen);
  if (stream->Read(jsonBuf.data(), jsonLen) != jsonLen)
  {
    vtkErrorMacro("Failed to read LYS JSON manifest");
    return 0;
  }

  nlohmann::json manifest;
  try
  {
    manifest = nlohmann::json::parse(jsonBuf.begin(), jsonBuf.end());
  }
  catch (const nlohmann::json::exception& e)
  {
    vtkErrorMacro("Failed to parse LYS JSON manifest: " << e.what());
    return 0;
  }

  if (!manifest.contains("mangoFiles"))
  {
    vtkErrorMacro("LYS JSON manifest has no 'mangoFiles' key");
    return 0;
  }
  const auto& mangoFiles = manifest["mangoFiles"];
  std::string geomKey;
  uint64_t geomOffset = 0;
  uint64_t geomSize = 0;

  // The geometry entry is the first key ending in ".bin" (with at least one character before
  // the extension) that is not "scene.bin". scene.bin holds scene metadata, not mesh geometry.
  for (auto it = mangoFiles.begin(); it != mangoFiles.end(); ++it)
  {
    const std::string& key = it.key();
    if (key.size() > 4 && key.ends_with(".bin") && key != "scene.bin")
    {
      geomKey = key;
      // offset field is a JSON string (e.g. "offset": "0"), verified across multiple .lys files.
      // size field is a JSON integer.
      geomOffset = std::stoull(it.value()["offset"].get<std::string>());
      geomSize = it.value()["size"].get<uint64_t>();
      break;
    }
  }

  if (geomKey.empty())
  {
    vtkErrorMacro("No geometry .bin entry found in LYS mangoFiles");
    return 0;
  }
  // Seek to the geometry data
  // Data section starts right after the 16-byte container header + padded JSON block.
  // We use jsonBlockSize (the padded size) to seek correctly past alignment bytes.
  // Geometry blob starts at: 16 + jsonBlockSize + geomOffset
  const vtkTypeInt64 dataSection =
    static_cast<vtkTypeInt64>(16) + static_cast<vtkTypeInt64>(jsonBlockSize);
  stream->Seek(
    dataSection + static_cast<vtkTypeInt64>(geomOffset), vtkResourceStream::SeekDirection::Begin);
  // Read the raw geometry blob
  std::vector<unsigned char> geomBuf(geomSize);
  if (stream->Read(geomBuf.data(), geomSize) != geomSize)
  {
    vtkErrorMacro("Failed to read LYS geometry blob");
    return 0;
  }
  // Parse the 12-byte mesh header:
  //   bytes  0- 3: indexCount  (uint32 LE) - number of uint32 indices
  //   bytes  4- 7: coordCount  (uint32 LE) - number of float32 values (nVertices * 3)
  //   bytes  8-11: reserved    (uint32 LE) - padding/flags, ignored
  // The index buffer follows immediately at byte 12.
  // The vertex buffer follows the index buffer.
  if (geomSize < sizeof(MeshHeader))
  {
    vtkErrorMacro("LYS geometry blob is too small to contain a mesh header");
    return 0;
  }

  const auto* meshHeader = reinterpret_cast<const MeshHeader*>(geomBuf.data());
  const uint32_t rawIndexCount = meshHeader->indexCount;
  if (rawIndexCount % 3 != 0)
  {
    vtkErrorMacro("LYS index count "
      << rawIndexCount << " is not a multiple of 3; index buffer is truncated or corrupted.");
    return 0;
  }
  const uint32_t indexCount = rawIndexCount;
  const uint32_t coordCount = meshHeader->coordCount;

  const size_t indexBufSize = static_cast<size_t>(indexCount) * sizeof(uint32_t);
  const size_t coordBufSize = static_cast<size_t>(coordCount) * sizeof(float);

  if (12 + indexBufSize + coordBufSize > geomSize)
  {
    vtkErrorMacro("LYS geometry blob is truncated (header claims more data than available)");
    return 0;
  }

  // Build vtkPoints from the vertex buffer.
  // Vertex buffer starts at byte 12 + indexBufSize.
  // Layout: XYZ float32 triplets, coordCount floats total = coordCount/3 points.
  const float* coords = reinterpret_cast<const float*>(geomBuf.data() + 12 + indexBufSize);
  const vtkIdType nPoints = static_cast<vtkIdType>(coordCount / 3);

  vtkNew<vtkFloatArray> pointArray;
  pointArray->SetNumberOfComponents(3);
  pointArray->SetNumberOfTuples(nPoints);
  std::memcpy(pointArray->GetPointer(0), coords, coordCount * sizeof(float));

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(pointArray);
  output->SetPoints(points);

  // Build vtkCellArray from the index buffer.
  // Index buffer starts at byte 12, every 3 indices form one triangle.
  // Normals are not stored in the file.
  const uint32_t* indices = reinterpret_cast<const uint32_t*>(geomBuf.data() + 12);
  const vtkIdType nTriangles = static_cast<vtkIdType>(indexCount / 3);

  // Check all indices are within bounds before building the cell array.
  for (vtkIdType t = 0; t < nTriangles; ++t)
  {
    if (static_cast<vtkIdType>(indices[t * 3 + 0]) >= nPoints ||
      static_cast<vtkIdType>(indices[t * 3 + 1]) >= nPoints ||
      static_cast<vtkIdType>(indices[t * 3 + 2]) >= nPoints)
    {
      vtkErrorMacro("LYS triangle " << t << " has out-of-bounds vertex indices");
      return 0;
    }
  }

  // Build offsets and connectivity arrays for vtkCellArray::SetData.
  vtkNew<vtkIdTypeArray> offsets;
  offsets->SetNumberOfValues(nTriangles + 1);
  for (vtkIdType i = 0; i <= nTriangles; ++i)
  {
    offsets->SetValue(i, i * 3);
  }

  vtkNew<vtkIdTypeArray> connectivity;
  connectivity->SetNumberOfValues(static_cast<vtkIdType>(indexCount));
  for (uint32_t i = 0; i < indexCount; ++i)
  {
    connectivity->SetValue(static_cast<vtkIdType>(i), static_cast<vtkIdType>(indices[i]));
  }

  vtkNew<vtkCellArray> triangles;
  triangles->SetData(offsets, connectivity);
  output->SetPolys(triangles);

  return 1;
}
//------------------------------------------------------------------------------
bool vtkF3DLYSReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }
  // Read the 16-byte container header
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  ContainerHeader header;
  if (stream->Read(&header, sizeof(ContainerHeader)) != sizeof(ContainerHeader))
  {
    return false;
  }
  const uint32_t jsonBlockSize = header.jsonBlockSize; // padded block size
  const uint32_t jsonLen = header.jsonLen;             // actual JSON content length
  if (jsonBlockSize < 2 || jsonLen < 2 || jsonLen > jsonBlockSize)
  {
    return false;
  }
  // Read and parse the JSON block, check for the "mangoFiles" key.
  std::vector<char> jsonBuf(jsonLen);
  if (stream->Read(jsonBuf.data(), jsonLen) != jsonLen)
  {
    return false;
  }
  try
  {
    const auto json = nlohmann::json::parse(jsonBuf.begin(), jsonBuf.end());
    return json.contains("mangoFiles");
  }
  catch (const nlohmann::json::exception&)
  {
    return false;
  }
}
