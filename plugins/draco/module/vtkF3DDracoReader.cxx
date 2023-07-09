#include "vtkF3DDracoReader.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include "draco/compression/decode.h"
#include "draco/draco_features.h"
#include "draco/io/stdio_file_reader.h"

#ifndef DRACO_MESH_COMPRESSION_SUPPORTED
#error "Please rebuild draco with DRACO_MESH_COMPRESSION cmake option enabled."
#endif
class vtkF3DDracoReader::vtkInternals
{
public:
  //----------------------------------------------------------------------------
  vtkInternals(vtkF3DDracoReader* parent)
    : Parent(parent)
  {
  }

  template<typename T>
  static vtkSmartPointer<vtkAOSDataArrayTemplate<T> > FillArray(
    int nbPoints, draco::PointAttribute* attribute)
  {
    vtkNew<vtkAOSDataArrayTemplate<T> > arr;

    arr->SetNumberOfComponents(attribute->num_components());
    arr->SetNumberOfTuples(nbPoints);

    for (int i = 0; i < nbPoints; i++)
    {
      draco::AttributeValueIndex idx = attribute->mapped_index(draco::PointIndex(i));
      T* p = reinterpret_cast<T*>(attribute->buffer()->data() +
        attribute->byte_stride() * idx.value() + attribute->byte_offset());
      arr->SetTypedTuple(i, p);
    }

    return arr;
  }

  template<typename T>
  static void FillPoints(const T& input, vtkPolyData* output)
  {
    int nbAttr = input->num_attributes();
    int nbPoints = input->num_points();

    for (int i = 0; i < nbAttr; i++)
    {
      draco::PointAttribute* attr = input->attribute(i);

      vtkSmartPointer<vtkDataArray> dataArray;

      switch (attr->data_type())
      {
        case draco::DT_INT8:
          dataArray = vtkInternals::FillArray<char>(nbPoints, attr);
          break;
        case draco::DT_UINT8:
          dataArray = vtkInternals::FillArray<unsigned char>(nbPoints, attr);
          break;
        case draco::DT_INT16:
          dataArray = vtkInternals::FillArray<short>(nbPoints, attr);
          break;
        case draco::DT_UINT16:
          dataArray = vtkInternals::FillArray<unsigned short>(nbPoints, attr);
          break;
        case draco::DT_INT32:
          dataArray = vtkInternals::FillArray<int>(nbPoints, attr);
          break;
        case draco::DT_UINT32:
          dataArray = vtkInternals::FillArray<unsigned int>(nbPoints, attr);
          break;
        case draco::DT_INT64:
          dataArray = vtkInternals::FillArray<long long>(nbPoints, attr);
          break;
        case draco::DT_UINT64:
          dataArray = vtkInternals::FillArray<unsigned long long>(nbPoints, attr);
          break;
        case draco::DT_FLOAT32:
          dataArray = vtkInternals::FillArray<float>(nbPoints, attr);
          break;
          break;
        case draco::DT_FLOAT64:
          dataArray = vtkInternals::FillArray<double>(nbPoints, attr);
          break;
        default:
          break;
      }

      if (dataArray)
      {
        switch (attr->attribute_type())
        {
          case draco::GeometryAttribute::Type::POSITION:
          {
            vtkNew<vtkPoints> points;
            points->SetData(dataArray);
            output->SetPoints(points);
            dataArray->SetName("Positions");
          }
          break;
          case draco::GeometryAttribute::Type::NORMAL:
            output->GetPointData()->SetNormals(dataArray);
            dataArray->SetName("Normals");
            break;
          case draco::GeometryAttribute::Type::COLOR:
            output->GetPointData()->SetScalars(dataArray);
            dataArray->SetName("Colors");
            break;
          case draco::GeometryAttribute::Type::TEX_COORD:
            output->GetPointData()->SetTCoords(dataArray);
            dataArray->SetName("TCoords");
            break;
          default:
            output->GetPointData()->AddArray(dataArray);
            dataArray->SetName((std::string("Generic#") + std::to_string(i)).c_str());
            break;
        }
      }
    }
  }

  static void FillFaces(const std::unique_ptr<draco::Mesh>& mesh, vtkPolyData* output)
  {
    int nbCells = mesh->num_faces();

    vtkNew<vtkIdTypeArray> triangles;
    triangles->SetNumberOfTuples(4 * nbCells);

    for (int i = 0; i < nbCells; i++)
    {
      const draco::Mesh::Face& face = mesh->face(draco::FaceIndex(i));

      triangles->SetTypedComponent(4 * i, 0, 3);
      for (int j = 0; j < 3; j++)
      {
        triangles->SetTypedComponent(4 * i + j + 1, 0, face[j].value());
      }
    }

    vtkNew<vtkCellArray> cells;
    cells->SetCells(nbCells, triangles);

    output->SetPolys(cells);
  }

  vtkF3DDracoReader* Parent;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DDracoReader);

//----------------------------------------------------------------------------
vtkF3DDracoReader::vtkF3DDracoReader()
  : Internals(new vtkF3DDracoReader::vtkInternals(this))
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DDracoReader::~vtkF3DDracoReader() = default;

//----------------------------------------------------------------------------
int vtkF3DDracoReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  std::vector<char> data;

  auto reader = draco::StdioFileReader::Open(this->FileName);
  if (!reader->ReadFileToBuffer(&data))
  {
    vtkErrorMacro("Cannot read file");
    return 0;
  }

  draco::DecoderBuffer buffer;
  buffer.Init(data.data(), data.size());

  draco::Decoder decoder;
  auto geom_type = draco::Decoder::GetEncodedGeometryType(&buffer);

  if (geom_type.value() == draco::TRIANGULAR_MESH)
  {
    auto mesh = decoder.DecodeMeshFromBuffer(&buffer);
    if (!mesh.value())
    {
      vtkErrorMacro("Cannot decode triangular mesh file");
      return 0;
    }

    vtkInternals::FillPoints(mesh.value(), output);
    vtkInternals::FillFaces(mesh.value(), output);
  }
  else if (geom_type.value() == draco::POINT_CLOUD)
  {
    auto pc = decoder.DecodePointCloudFromBuffer(&buffer);
    if (!pc.value())
    {
      vtkErrorMacro("Cannot decode point cloud file");
      return 0;
    }

    vtkInternals::FillPoints(pc.value(), output);
  }

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DDracoReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
