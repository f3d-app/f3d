#include "vtkF3DIFCReader.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkFileResourceStream.h>
#include <vtkResourceStream.h>
#include <vtkUnsignedCharArray.h>

#include <web-ifc/modelmanager/ModelManager.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>

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

namespace
{
//----------------------------------------------------------------------------
std::string ReadStreamContent(vtkResourceStream* stream)
{
  if (!stream)
  {
    return {};
  }

  std::ostringstream oss;
  constexpr size_t bufferSize = 65536;
  std::vector<char> buffer(bufferSize);

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  while (true)
  {
    size_t bytesRead = stream->Read(buffer.data(), bufferSize);
    if (bytesRead == 0)
    {
      break;
    }
    oss.write(buffer.data(), bytesRead);
  }

  return oss.str();
}
}

//----------------------------------------------------------------------------
int vtkF3DIFCReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  std::string content;
  if (this->Stream)
  {
    content = ::ReadStreamContent(this->Stream);
  }
  else if (!this->FileName.empty())
  {
    vtkNew<vtkFileResourceStream> fileStream;
    if (!fileStream->Open(this->FileName.c_str()))
    {
      vtkErrorMacro("Failed to open file: " << this->FileName);
      return 0;
    }
    content = ::ReadStreamContent(fileStream);
  }

  if (content.empty())
  {
    vtkErrorMacro("Failed to read IFC content");
    return 0;
  }

  try
  {
    webifc::manager::LoaderSettings settings;
    settings.CIRCLE_SEGMENTS = static_cast<uint16_t>(this->CircleSegments);
    settings.COORDINATE_TO_ORIGIN = false;

    uint32_t modelID = this->Internals->Manager.CreateModel(settings);
    webifc::parsing::IfcLoader* loader = this->Internals->Manager.GetIfcLoader(modelID);

    loader->LoadFile([&content](char* dest, size_t sourceOffset, size_t destSize)
      {
        if (sourceOffset >= content.size())
        {
          return static_cast<uint32_t>(0);
        }
        size_t length = std::min(content.size() - sourceOffset, destSize);
        memcpy(dest, content.data() + sourceOffset, length);
        return static_cast<uint32_t>(length);
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
        auto& geometry = geometryProcessor->GetGeometry(placedGeom.geometryExpressID);

        const auto& vertexData = geometry.vertexData;
        const auto& indexData = geometry.indexData;

        if (vertexData.empty() || indexData.empty())
        {
          continue;
        }

        const auto& transform = placedGeom.flatTransformation;
        std::array<unsigned char, 4> color = {
          static_cast<unsigned char>(placedGeom.color.r * 255),
          static_cast<unsigned char>(placedGeom.color.g * 255),
          static_cast<unsigned char>(placedGeom.color.b * 255),
          static_cast<unsigned char>(placedGeom.color.a * 255)
        };

        vtkIdType pointOffset = allPoints->GetNumberOfPoints();
        constexpr int vertexSize = 6;

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
          normals->InsertNextTuple3(static_cast<float>(tnx), static_cast<float>(tny),
            static_cast<float>(tnz));
        }

        for (size_t i = 0; i < indexData.size(); i += 3)
        {
          vtkIdType pts[3] = {
            static_cast<vtkIdType>(indexData[i]) + pointOffset,
            static_cast<vtkIdType>(indexData[i + 1]) + pointOffset,
            static_cast<vtkIdType>(indexData[i + 2]) + pointOffset
          };
          allPolys->InsertNextCell(3, pts);
          colors->InsertNextTypedTuple(color.data());
        }
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
