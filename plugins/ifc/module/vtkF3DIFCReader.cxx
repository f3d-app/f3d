#include "vtkF3DIFCReader.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkUnsignedCharArray.h>

#include <web-ifc/modelmanager/ModelManager.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

vtkStandardNewMacro(vtkF3DIFCReader);

//----------------------------------------------------------------------------
class vtkF3DIFCReader::vtkInternals
{
public:
  webifc::manager::ModelManager Manager{ false };
};

//----------------------------------------------------------------------------
vtkF3DIFCReader::vtkF3DIFCReader()
  : Internals(new vtkInternals())
{
  this->vtkAlgorithm::SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DIFCReader::~vtkF3DIFCReader() = default;

//----------------------------------------------------------------------------
void vtkF3DIFCReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "CircleSegments: " << this->CircleSegments << "\n";
  os << indent << "ReadOpenings: " << (this->ReadOpenings ? "true" : "false") << "\n";
  os << indent << "ReadSpaces: " << (this->ReadSpaces ? "true" : "false") << "\n";
}

//----------------------------------------------------------------------------
void vtkF3DIFCReader::SetStream(vtkResourceStream* stream)
{
  if (this->Stream != stream)
  {
    this->Stream = stream;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
vtkResourceStream* vtkF3DIFCReader::GetStream()
{
  return this->Stream;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkF3DIFCReader::GetMTime()
{
  vtkMTimeType mtime = this->Superclass::GetMTime();
  if (this->Stream)
  {
    mtime = std::max(mtime, this->Stream->GetMTime());
  }
  return mtime;
}

//----------------------------------------------------------------------------
bool vtkF3DIFCReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  constexpr std::string_view ifcHeader{ "ISO-10303-21;", 13 };
  std::array<char, 13> buffer;

  if (stream->Read(&buffer, buffer.size()) != buffer.size())
  {
    return false;
  }

  return std::string_view(buffer.data(), buffer.size()) == ifcHeader;
}

//----------------------------------------------------------------------------
int vtkF3DIFCReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  vtkSmartPointer<vtkResourceStream> stream;
  if (this->Stream)
  {
    stream = this->Stream;
  }
  else if (!this->FileName.empty())
  {
    vtkNew<vtkFileResourceStream> fileStream;
    if (!fileStream->Open(this->FileName.c_str()))
    {
      vtkErrorMacro("Failed to open file: " << this->FileName);
      return 0;
    }
    stream = fileStream;
  }

  if (!stream)
  {
    vtkErrorMacro("No input stream or filename specified");
    return 0;
  }

  try
  {
    webifc::manager::LoaderSettings settings;
    settings.CIRCLE_SEGMENTS = static_cast<uint16_t>(this->CircleSegments);
    settings.COORDINATE_TO_ORIGIN = false;

    auto oldLevel = spdlog::get_level();
    spdlog::set_level(spdlog::level::warn);
    uint32_t modelID = this->Internals->Manager.CreateModel(settings);
    spdlog::set_level(oldLevel);
    webifc::parsing::IfcLoader* loader = this->Internals->Manager.GetIfcLoader(modelID);

    loader->LoadFile(
      [&stream](char* dest, size_t sourceOffset, size_t destSize)
      {
        stream->Seek(sourceOffset, vtkResourceStream::SeekDirection::Begin);
        return static_cast<uint32_t>(stream->Read(dest, destSize));
      });

    webifc::geometry::IfcGeometryProcessor* geometryProcessor =
      this->Internals->Manager.GetGeometryProcessor(modelID);

    const webifc::schema::IfcSchemaManager& schemaManager =
      this->Internals->Manager.GetSchemaManager();

    vtkNew<vtkPoints> allPoints;
    vtkNew<vtkCellArray> allPolys;
    vtkNew<vtkFloatArray> normals;
    normals->SetNumberOfComponents(3);
    normals->SetName("Normals");
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(4);
    colors->SetName("Colors");

    auto processElement = [&](uint32_t expressID)
    {
      auto mesh = geometryProcessor->GetFlatMesh(expressID);

      for (const auto& placedGeom : mesh.geometries)
      {
        const auto& geometry = geometryProcessor->GetGeometry(placedGeom.geometryExpressID);

        const auto& vertexData = geometry.vertexData;
        const auto& indexData = geometry.indexData;

        if (vertexData.empty() || indexData.empty())
        {
          continue;
        }

        const auto& transform = placedGeom.flatTransformation;
        std::array<unsigned char, 4> color = { static_cast<unsigned char>(placedGeom.color.r * 255),
          static_cast<unsigned char>(placedGeom.color.g * 255),
          static_cast<unsigned char>(placedGeom.color.b * 255),
          static_cast<unsigned char>(placedGeom.color.a * 255) };

        vtkIdType pointOffset = allPoints->GetNumberOfPoints();
        constexpr int vertexSize = 6;

#ifdef __linux__
        // WORKAROUND: web-ifc produces inconsistent triangle winding order on Linux.
        // Vertex normals are correct, so compare with geometric normal to fix winding.
        // Once https://github.com/ThatOpen/engine_web-ifc/issues/1811 is fixed,
        // remove this #ifdef __linux__ block and keep only the #else branch.
        std::vector<std::array<double, 3>> localPositions;
        std::vector<std::array<double, 3>> localNormals;
        localPositions.reserve(vertexData.size() / vertexSize);
        localNormals.reserve(vertexData.size() / vertexSize);

        for (size_t i = 0; i < vertexData.size(); i += vertexSize)
        {
          double x = vertexData[i];
          double y = vertexData[i + 1];
          double z = vertexData[i + 2];
          double nx = vertexData[i + 3];
          double ny = vertexData[i + 4];
          double nz = vertexData[i + 5];

          double tx = transform[0] * x + transform[4] * y + transform[8] * z + transform[12];
          double ty = transform[1] * x + transform[5] * y + transform[9] * z + transform[13];
          double tz = transform[2] * x + transform[6] * y + transform[10] * z + transform[14];

          double tnx = transform[0] * nx + transform[4] * ny + transform[8] * nz;
          double tny = transform[1] * nx + transform[5] * ny + transform[9] * nz;
          double tnz = transform[2] * nx + transform[6] * ny + transform[10] * nz;

          localPositions.push_back({ tx, ty, tz });
          localNormals.push_back({ tnx, tny, tnz });
          allPoints->InsertNextPoint(tx, ty, tz);
          normals->InsertNextTuple3(
            static_cast<float>(tnx), static_cast<float>(tny), static_cast<float>(tnz));
        }

        for (size_t i = 0; i < indexData.size(); i += 3)
        {
          uint32_t i0 = indexData[i];
          uint32_t i1 = indexData[i + 1];
          uint32_t i2 = indexData[i + 2];

          const auto& p0 = localPositions[i0];
          const auto& p1 = localPositions[i1];
          const auto& p2 = localPositions[i2];

          double e1x = p1[0] - p0[0], e1y = p1[1] - p0[1], e1z = p1[2] - p0[2];
          double e2x = p2[0] - p0[0], e2y = p2[1] - p0[1], e2z = p2[2] - p0[2];
          double gnx = e1y * e2z - e1z * e2y;
          double gny = e1z * e2x - e1x * e2z;
          double gnz = e1x * e2y - e1y * e2x;

          const auto& n0 = localNormals[i0];
          const auto& n1 = localNormals[i1];
          const auto& n2 = localNormals[i2];
          double avgNx = n0[0] + n1[0] + n2[0];
          double avgNy = n0[1] + n1[1] + n2[1];
          double avgNz = n0[2] + n1[2] + n2[2];

          bool windingIsWrong = (gnx * avgNx + gny * avgNy + gnz * avgNz) < 0;
          if (windingIsWrong)
          {
            std::swap(i1, i2);
          }

          vtkIdType pts[3] = { static_cast<vtkIdType>(i0) + pointOffset,
            static_cast<vtkIdType>(i1) + pointOffset, static_cast<vtkIdType>(i2) + pointOffset };
          allPolys->InsertNextCell(3, pts);
          colors->InsertNextTypedTuple(color.data());
        }
#else
        for (size_t i = 0; i < vertexData.size(); i += vertexSize)
        {
          double x = vertexData[i];
          double y = vertexData[i + 1];
          double z = vertexData[i + 2];
          double nx = vertexData[i + 3];
          double ny = vertexData[i + 4];
          double nz = vertexData[i + 5];

          double tx = transform[0] * x + transform[4] * y + transform[8] * z + transform[12];
          double ty = transform[1] * x + transform[5] * y + transform[9] * z + transform[13];
          double tz = transform[2] * x + transform[6] * y + transform[10] * z + transform[14];

          double tnx = transform[0] * nx + transform[4] * ny + transform[8] * nz;
          double tny = transform[1] * nx + transform[5] * ny + transform[9] * nz;
          double tnz = transform[2] * nx + transform[6] * ny + transform[10] * nz;

          allPoints->InsertNextPoint(tx, ty, tz);
          normals->InsertNextTuple3(
            static_cast<float>(tnx), static_cast<float>(tny), static_cast<float>(tnz));
        }

        for (size_t i = 0; i < indexData.size(); i += 3)
        {
          vtkIdType pts[3] = { static_cast<vtkIdType>(indexData[i]) + pointOffset,
            static_cast<vtkIdType>(indexData[i + 1]) + pointOffset,
            static_cast<vtkIdType>(indexData[i + 2]) + pointOffset };
          allPolys->InsertNextCell(3, pts);
          colors->InsertNextTypedTuple(color.data());
        }
#endif
      }
    };

    auto elementTypes = schemaManager.GetIfcElementList();

    for (const auto& type : elementTypes)
    {
      std::string typeStr = std::string(schemaManager.IfcTypeCodeToType(type));

      if (!this->ReadOpenings && typeStr == "IfcOpeningElement")
      {
        continue;
      }
      if (!this->ReadSpaces && typeStr == "IfcSpace")
      {
        continue;
      }

      auto elements = loader->GetExpressIDsWithType(type);
      for (auto expressID : elements)
      {
        processElement(expressID);
      }
    }

    output->SetPoints(allPoints);
    output->SetPolys(allPolys);
    output->GetPointData()->SetNormals(normals);
    output->GetCellData()->SetScalars(colors);

    this->Internals->Manager.CloseModel(modelID);
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro("Error processing IFC file: " << e.what());
    return 0;
  }

  return 1;
}
