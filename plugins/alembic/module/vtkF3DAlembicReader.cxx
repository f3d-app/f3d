#include "vtkF3DAlembicReader.h"

#include <vtkAppendPolyData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include <Alembic/Abc/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/Util/All.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using IndicesContainer = std::vector<int32_t>;
using V3fContainer = std::vector<Alembic::Abc::V3f>;
using AttributesContainer = std::map<std::string, V3fContainer>;
using PerFaceWavefrontIndicesTripletsContainer = std::vector<Alembic::Abc::V3i>;
using PerMeshWavefrontIndicesTripletsContainer =
  std::vector<PerFaceWavefrontIndicesTripletsContainer>;

const size_t P_indices_offset = 0;
const size_t uv_indices_offset = 1;
const size_t N_indices_offset = 2;

struct IntermediateGeometry
{
  AttributesContainer _attributes;
  PerMeshWavefrontIndicesTripletsContainer _indices;
  bool _uv_is_facevarying{false};
  bool _N_is_facevarying{false};
};

class vtkF3DAlembicReader::vtkInternals
{
  void SetupIndicesStorage(const Alembic::AbcGeom::Int32ArraySamplePtr& face_vertex_counts,
    PerMeshWavefrontIndicesTripletsContainer& extracted_indices)
  {

    for (size_t i = 0; i < face_vertex_counts->size(); i++)
    {
      auto this_face_vertex_count = face_vertex_counts->get()[i];
      PerFaceWavefrontIndicesTripletsContainer per_face_indices(this_face_vertex_count);
      extracted_indices.push_back(per_face_indices);
    }
  }

  template<typename I>
  void UpdateIndices(const I& attribute_indices, size_t indices_offset, bool do_reverse_rotate,
    PerMeshWavefrontIndicesTripletsContainer& mesh_indices)
  {
    auto face_indices_counter = 0;
    for (auto & per_face_indices : mesh_indices)
    {
      auto this_face_vertex_count = per_face_indices.size();
      IndicesContainer this_face_indices;
      // Perform the collection first
      for (size_t j = 0; j < this_face_vertex_count; j++)
      {
        auto vertex = attribute_indices->get()[face_indices_counter];
        // mesh_indices[i][j][indices_offset] = vertex;
        this_face_indices.emplace_back(vertex);
        // printf("vertex[%ld] %d\n", indices_offset, vertex + 1);
        face_indices_counter++;
      }
      if (do_reverse_rotate)
      {
        std::reverse(this_face_indices.begin(), this_face_indices.end());
        std::rotate(this_face_indices.begin(),
          this_face_indices.begin() + this_face_indices.size() - 1, this_face_indices.end());
      }
      // Now update the mesh's indices
      for (size_t j = 0; j < this_face_vertex_count; j++)
      {
        per_face_indices[j][indices_offset] = this_face_indices[j];
        // printf("reversed rotated vertex %d\n",this_face_indices[j]+1);
      }
    }
  }

  void PointDuplicateAccumulator(
    const IntermediateGeometry& original_data, IntermediateGeometry& duplicated_data)
  {
    bool need_to_duplicate = original_data._uv_is_facevarying || original_data._N_is_facevarying;

    auto uv_map_iter = original_data._attributes.find("uv");
    auto N_map_iter = original_data._attributes.find("N");
    bool have_uv = uv_map_iter != original_data._attributes.end();
    bool have_N = N_map_iter != original_data._attributes.end();

    if (need_to_duplicate)
    {
      auto face_count = original_data._indices.size();
      duplicated_data._indices.resize(face_count);
      for (size_t i = 0; i < face_count; i++)
      {
        auto this_face_vertex_count = original_data._indices[i].size();
        duplicated_data._indices[i].resize(
          this_face_vertex_count, Alembic::Abc::V3i(-9999, -9999, -9999));
      }

      // Points
      {
        V3fContainer P_v3f;
        int P_running_index = 0;
        for (size_t i = 0; i < face_count; i++)
        {
          auto this_face_vertex_count = original_data._indices[i].size();
          for (size_t j = 0; j < this_face_vertex_count; j++)
          {
            Alembic::Abc::V3f original_position =
              original_data._attributes.at("P")[original_data._indices[i][j].x];
            P_v3f.emplace_back(original_position);
            duplicated_data._indices[i][j].x = P_running_index;
            P_running_index++;
          }
        }

        duplicated_data._attributes.insert(AttributesContainer::value_type("P", P_v3f));
      }

      // UV
      if (have_uv)
      {
        V3fContainer uv_v3f;
        int uv_running_index = 0;

        for (size_t i = 0; i < face_count; i++)
        {
          auto this_face_vertex_count = original_data._indices[i].size();
          for (size_t j = 0; j < this_face_vertex_count; j++)
          {
            Alembic::Abc::V3f original_uv =
              original_data._attributes.at("uv")[original_data._indices[i][j].y];
            uv_v3f.emplace_back(original_uv);
            duplicated_data._indices[i][j].y = uv_running_index;
            uv_running_index++;
          }
        }

        duplicated_data._attributes.insert(AttributesContainer::value_type("uv", uv_v3f));
      }

      // Normal
      if (have_N)
      {
        V3fContainer N_v3f;
        int N_running_index = 0;

        for (size_t i = 0; i < face_count; i++)
        {
          auto this_face_vertex_count = original_data._indices[i].size();
          for (size_t j = 0; j < this_face_vertex_count; j++)
          {
            Alembic::Abc::V3f original_N =
              original_data._attributes.at("N")[original_data._indices[i][j].z];
            N_v3f.emplace_back(original_N);
            duplicated_data._indices[i][j].z = N_running_index;
            N_running_index++;
          }
        }

        duplicated_data._attributes.insert(AttributesContainer::value_type("N", N_v3f));
      }
    }
    else
    {
      duplicated_data = original_data;
    }
  }

  void WriteVTP(const IntermediateGeometry& data, vtkPolyData* polydata)
  {
    // Create 10 points.
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> cells;

    auto P_map_iter = data._attributes.find("P");
    assert(P_map_iter != data._attributes.end());
    // Note : uv and N are optional
    auto uv_map_iter = data._attributes.find("uv");
    auto N_map_iter = data._attributes.find("N");
    bool have_uv = uv_map_iter != data._attributes.end();
    bool have_N = N_map_iter != data._attributes.end();
    for (auto& P_iter : P_map_iter->second)
    {
      points->InsertNextPoint(P_iter.x, P_iter.y, P_iter.z);
    }
    polydata->SetPoints(points);

    for (auto& face_indices_iter : data._indices)
    {
      vtkNew<vtkPolygon> polygon;
      for (auto& vertex_indices_iter : face_indices_iter)
      {
        polygon->GetPointIds()->InsertNextId(vertex_indices_iter.x);
      }
      cells->InsertNextCell(polygon);
    }
    polydata->SetPolys(cells);
    vtkDataSetAttributes* point_attributes = polydata->GetAttributes(vtkDataSet::POINT);
    vtkDataSetAttributes* cell_attributes = polydata->GetAttributes(vtkDataSet::CELL);

    assert(point_attributes != nullptr);
    assert(cell_attributes != nullptr);
    if (have_N)
    {
      vtkNew<vtkFloatArray> normals;
      normals->SetNumberOfComponents(3);
      for (auto& N : N_map_iter->second)
      {
        normals->InsertNextTuple3(N.x, N.y, N.z);
      }
      if (data._N_is_facevarying)
      {
        cell_attributes->SetNormals(normals);
      }
      else
      {
        point_attributes->SetNormals(normals);
      }
    }

    if (have_uv)
    {
      vtkNew<vtkFloatArray> uvs;
      uvs->SetNumberOfComponents(2);
      for (auto& uv : uv_map_iter->second)
      {
        uvs->InsertNextTuple2(uv.x, uv.y);
      }
      if (data._uv_is_facevarying)
      {
        cell_attributes->SetTCoords(uvs);
      }
      else
      {
        point_attributes->SetTCoords(uvs);
      }
    }
  }

public:
  vtkSmartPointer<vtkPolyData> ProcessIPolyMesh(
    const Alembic::AbcGeom::IPolyMesh& pmesh)
  {
    vtkNew<vtkPolyData> polydata;
    IntermediateGeometry original_data;

    Alembic::AbcGeom::IPolyMeshSchema::Sample samp;
    const Alembic::AbcGeom::IPolyMeshSchema& schema = pmesh.getSchema();
    if (schema.getNumSamples() > 0)
    {

      schema.get(samp);

      Alembic::AbcGeom::P3fArraySamplePtr P = samp.getPositions();
      Alembic::AbcGeom::Int32ArraySamplePtr face_position_indices = samp.getFaceIndices();
      Alembic::AbcGeom::Int32ArraySamplePtr face_vertex_counts = samp.getFaceCounts();

      SetupIndicesStorage(face_vertex_counts, original_data._indices);

      // Position
      {
        V3fContainer P_v3f;
        for (size_t P_index = 0; P_index < P->size(); P_index++)
        {
          P_v3f.emplace_back(P->get()[P_index].x, P->get()[P_index].y, P->get()[P_index].z);
        }
        original_data._attributes.insert(AttributesContainer::value_type("P", P_v3f));

        UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
          face_position_indices, P_indices_offset, true, original_data._indices);
      }

      // Texture coordinate
      Alembic::AbcGeom::IV2fGeomParam uvsParam = schema.getUVsParam();
      if (uvsParam.valid())
      {
        Alembic::AbcGeom::IV2fGeomParam::Sample uvValue = uvsParam.getIndexedValue();
        if (uvValue.valid())
        {
          V3fContainer uv_v3f;
          Alembic::AbcGeom::UInt32ArraySamplePtr uv_indices = uvValue.getIndices();
          for (size_t index = 0; index < uvValue.getVals()->size(); ++index)
          {
            Alembic::AbcGeom::V2f uv = (*(uvValue.getVals()))[index];
            uv_v3f.emplace_back(uv[0], uv[1], 0);
          }
          original_data._attributes.insert(AttributesContainer::value_type("uv", uv_v3f));

          if (uvsParam.getScope() == Alembic::AbcGeom::kFacevaryingScope)
          {
            original_data._uv_is_facevarying = true;
            UpdateIndices<Alembic::AbcGeom::UInt32ArraySamplePtr>(
              uv_indices, uv_indices_offset, true, original_data._indices);
          }
          else
          {
            UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
              face_position_indices, uv_indices_offset, true, original_data._indices);
          }
        }
      }

      // Normals
      Alembic::AbcGeom::IN3fGeomParam normalsParam = schema.getNormalsParam();
      if (normalsParam.valid())
      {
        Alembic::AbcGeom::IN3fGeomParam::Sample normalValue = normalsParam.getIndexedValue();
        if (normalValue.valid())
        {
          V3fContainer normal_v3f;
          Alembic::AbcGeom::UInt32ArraySamplePtr normal_indices = normalValue.getIndices();
          for (size_t index = 0; index < normalValue.getVals()->size(); ++index)
          {
            Alembic::AbcGeom::V3f normal = (*(normalValue.getVals()))[index];
            normal_v3f.emplace_back(normal[0], normal[1], normal[2]);
          }
          original_data._attributes.insert(AttributesContainer::value_type("N", normal_v3f));

          if (normalsParam.getScope() == Alembic::AbcGeom::kFacevaryingScope)
          {
            original_data._N_is_facevarying = true;

            UpdateIndices<Alembic::AbcGeom::UInt32ArraySamplePtr>(
              normal_indices, N_indices_offset, true, original_data._indices);
          }
          else
          {
            UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
              face_position_indices, N_indices_offset, true, original_data._indices);
          }
        }
      }
    }

    IntermediateGeometry duplicated_data;

    PointDuplicateAccumulator(original_data, duplicated_data);

    WriteVTP(duplicated_data, polydata);

    return polydata;
  }

  void IterateIObject(vtkAppendPolyData* append, const Alembic::Abc::IObject& parent,
    const Alembic::Abc::ObjectHeader& ohead)
  {
    // Set this if we should continue traversing
    Alembic::Abc::IObject nextParentObject;

    if (Alembic::AbcGeom::IXform::matches(ohead))
    {
      Alembic::AbcGeom::IXform xform(parent, ohead.getName());
      nextParentObject = xform;
    }
    else if (Alembic::AbcGeom::IPolyMesh::matches(ohead))
    {
      Alembic::AbcGeom::IPolyMesh polymesh(parent, ohead.getName());
      append->AddInputData(ProcessIPolyMesh(polymesh));
      nextParentObject = polymesh;
    }

    // Recursion
    if (nextParentObject.valid())
    {
      for (size_t i = 0; i < nextParentObject.getNumChildren(); ++i)
      {
        this->IterateIObject(append, nextParentObject, nextParentObject.getChildHeader(i));
      }
    }
  }

  void ImportRoot(vtkAppendPolyData* append)
  {
    Alembic::Abc::IObject top = this->Archive.getTop();

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      this->IterateIObject(append, top, top.getChildHeader(i));
    }
  }

  void ReadArchive(const std::string& filePath)
  {
    Alembic::AbcCoreFactory::IFactory factory;
    Alembic::AbcCoreFactory::IFactory::CoreType coreType;

    this->Archive = factory.getArchive(filePath, coreType);
  }
  Alembic::Abc::IArchive Archive;
};

vtkStandardNewMacro(vtkF3DAlembicReader);

//----------------------------------------------------------------------------
vtkF3DAlembicReader::vtkF3DAlembicReader()
  : Internals(new vtkF3DAlembicReader::vtkInternals())
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DAlembicReader::~vtkF3DAlembicReader() = default;

//----------------------------------------------------------------------------
int vtkF3DAlembicReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  this->Internals->ReadArchive(this->FileName);

  vtkNew<vtkAppendPolyData> append;
  this->Internals->ImportRoot(append);
  append->Update();

  output->ShallowCopy(append->GetOutput());

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DAlembicReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
