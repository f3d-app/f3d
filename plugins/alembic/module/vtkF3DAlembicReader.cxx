#include "vtkF3DAlembicReader.h"

#include <vtkAppendPolyData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include <Alembic/Abc/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/Util/All.h>
#include <stack>
#include <tuple>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using IndicesContainer = std::vector<int>;
using V3fContainer = std::vector<Alembic::Abc::V3f>;
using AttributesContainer = std::map<std::string, V3fContainer>;
using PerFaceWavefrontIndicesTripletsContainer = std::vector<Alembic::Abc::V3i>;
using PerMeshWavefrontIndicesTripletsContainer =
  std::vector<PerFaceWavefrontIndicesTripletsContainer>;

constexpr int pIndicesOffset = 0;
constexpr int uvIndicesOffset = 1;
constexpr int nIndicesOffset = 2;

struct IntermediateGeometry
{
  AttributesContainer Attributes;
  PerMeshWavefrontIndicesTripletsContainer Indices;
  bool uvFaceVarying = false;
  bool nFaceVarying = false;
};

class vtkF3DAlembicReader::vtkInternals
{
  void SetupIndicesStorage(const Alembic::AbcGeom::Int32ArraySamplePtr& faceVertexCounts,
    PerMeshWavefrontIndicesTripletsContainer& extractedIndices)
  {
    for (size_t i = 0; i < faceVertexCounts->size(); i++)
    {
      extractedIndices.emplace_back(faceVertexCounts->get()[i]);
    }
  }

  template<typename I>
  void UpdateIndices(const I& attributeIndices, int indicesOffset,
    PerMeshWavefrontIndicesTripletsContainer& meshIndices, bool doReverseRotate = true)
  {
    size_t faceIndicesCounter = 0;
    for (auto& perFaceIndices : meshIndices)
    {
      // Perform the collection first
      size_t thisFaceVertexCount = perFaceIndices.size();
      IndicesContainer thisFaceIndices;
      for (size_t j = 0; j < thisFaceVertexCount; j++)
      {
        auto vertex = attributeIndices->get()[faceIndicesCounter];
        thisFaceIndices.emplace_back(vertex);
        faceIndicesCounter++;
      }
      if (doReverseRotate)
      {
        std::reverse(thisFaceIndices.begin(), thisFaceIndices.end());
        std::rotate(thisFaceIndices.begin(), thisFaceIndices.begin() + thisFaceIndices.size() - 1,
          thisFaceIndices.end());
      }

      // Now update the mesh's indices
      for (size_t j = 0; j < thisFaceVertexCount; j++)
      {
        perFaceIndices[j][indicesOffset] = thisFaceIndices[j];
      }
    }
  }

  void PointDuplicateAccumulator(
    const IntermediateGeometry& originalData, IntermediateGeometry& duplicatedData)
  {
    duplicatedData.uvFaceVarying = originalData.uvFaceVarying;
    duplicatedData.nFaceVarying = originalData.nFaceVarying;
    bool needToDuplicate = originalData.uvFaceVarying || originalData.nFaceVarying;

    auto uvMapIter = originalData.Attributes.find("uv");
    auto nMapIter = originalData.Attributes.find("N");
    bool haveUV = uvMapIter != originalData.Attributes.end();
    bool haveN = nMapIter != originalData.Attributes.end();

    if (needToDuplicate)
    {
      auto faceCount = originalData.Indices.size();
      duplicatedData.Indices.resize(faceCount);
      for (size_t i = 0; i < faceCount; i++)
      {
        auto thisFaceVertexCount = originalData.Indices[i].size();
        duplicatedData.Indices[i].resize(thisFaceVertexCount, Alembic::Abc::V3i());
      }

      // Points
      {
        V3fContainer pV3F;
        int pRunningIndex = 0;
        for (size_t i = 0; i < faceCount; i++)
        {
          auto thisFaceVertexCount = originalData.Indices[i].size();
          for (size_t j = 0; j < thisFaceVertexCount; j++)
          {
            Alembic::Abc::V3f originalPosition =
              originalData.Attributes.at("P")[originalData.Indices[i][j].x];
            pV3F.emplace_back(originalPosition);
            duplicatedData.Indices[i][j].x = pRunningIndex;
            pRunningIndex++;
          }
        }

        duplicatedData.Attributes.insert(AttributesContainer::value_type("P", pV3F));
      }

      // UV
      if (haveUV)
      {
        V3fContainer uvV3F;
        int uvRunningIndex = 0;

        for (size_t i = 0; i < faceCount; i++)
        {
          auto thisFaceVertexCount = originalData.Indices[i].size();
          for (size_t j = 0; j < thisFaceVertexCount; j++)
          {
            Alembic::Abc::V3f originalUV =
              originalData.Attributes.at("uv")[originalData.Indices[i][j].y];
            uvV3F.emplace_back(originalUV);
            duplicatedData.Indices[i][j].y = uvRunningIndex;
            uvRunningIndex++;
          }
        }

        duplicatedData.Attributes.insert(AttributesContainer::value_type("uv", uvV3F));
      }

      // Normal
      if (haveN)
      {
        V3fContainer nV3F;
        int nRunningIndex = 0;

        for (size_t i = 0; i < faceCount; i++)
        {
          auto thisFaceVertexCount = originalData.Indices[i].size();
          for (size_t j = 0; j < thisFaceVertexCount; j++)
          {
            Alembic::Abc::V3f originalN =
              originalData.Attributes.at("N")[originalData.Indices[i][j].z];
            nV3F.emplace_back(originalN);
            duplicatedData.Indices[i][j].z = nRunningIndex;
            nRunningIndex++;
          }
        }

        duplicatedData.Attributes.insert(AttributesContainer::value_type("N", nV3F));
      }
    }
    else
    {
      duplicatedData = originalData;
    }
  }

  void FillPolyData(const IntermediateGeometry& data, vtkPolyData* polydata)
  {
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> cells;

    auto pMapIter = data.Attributes.find("P");
    if (pMapIter == data.Attributes.end())
    {
      // Not a geometry, silent return
      return;
    }

    // Note : uv and N are optional
    auto uvMapIter = data.Attributes.find("uv");
    auto nMapIter = data.Attributes.find("N");
    bool haveUV = uvMapIter != data.Attributes.end();
    bool haveN = nMapIter != data.Attributes.end();
    for (auto& pIter : pMapIter->second)
    {
      points->InsertNextPoint(pIter.x, pIter.y, pIter.z);
    }
    polydata->SetPoints(points);

    std::vector<vtkIdType> indexArr;
    for (auto& faceIndicesIter : data.Indices)
    {
      indexArr.clear();

      std::transform(faceIndicesIter.cbegin(), faceIndicesIter.cend(), std::back_inserter(indexArr),
        [](const Alembic::Abc::V3i& v) { return v.x; });

      cells->InsertNextCell(indexArr.size(), indexArr.data());
    }
    polydata->SetPolys(cells);
    vtkDataSetAttributes* pointAttributes = polydata->GetAttributes(vtkDataSet::POINT);

    if (haveN)
    {
      vtkNew<vtkFloatArray> normals;
      normals->SetName("Normals");
      normals->SetNumberOfComponents(3);
      for (auto& N : nMapIter->second)
      {
        normals->InsertNextTuple3(N.x, N.y, N.z);
      }
      pointAttributes->SetNormals(normals);
    }

    if (haveUV)
    {
      vtkNew<vtkFloatArray> uvs;
      uvs->SetName("UVs");
      uvs->SetNumberOfComponents(2);
      for (auto& uv : uvMapIter->second)
      {
        uvs->InsertNextTuple2(uv.x, uv.y);
      }
      pointAttributes->SetTCoords(uvs);
    }
  }

public:
  vtkSmartPointer<vtkPolyData> ProcessIPolyMesh(
    const Alembic::AbcGeom::IPolyMesh& pmesh, double time, const Alembic::Abc::M44d& matrix)
  {
    vtkNew<vtkPolyData> polydata;
    IntermediateGeometry originalData;

    Alembic::AbcGeom::IPolyMeshSchema::Sample samp;
    const Alembic::AbcGeom::IPolyMeshSchema& schema = pmesh.getSchema();
    if (schema.getNumSamples() > 0)
    {
      Alembic::AbcGeom::ISampleSelector selector(time);
      schema.get(samp, selector);

      Alembic::AbcGeom::P3fArraySamplePtr positions = samp.getPositions();
      Alembic::AbcGeom::Int32ArraySamplePtr facePositionIndices = samp.getFaceIndices();
      Alembic::AbcGeom::Int32ArraySamplePtr faceVertexCounts = samp.getFaceCounts();

      this->SetupIndicesStorage(faceVertexCounts, originalData.Indices);

      // Positions
      {
        V3fContainer pV3F;
        for (size_t pIndex = 0; pIndex < positions->size(); pIndex++)
        {
          const Alembic::Abc::V3f tp = positions->get()[pIndex] * matrix;
          pV3F.emplace_back(tp.x, tp.y, tp.z);
        }
        originalData.Attributes.insert(AttributesContainer::value_type("P", pV3F));

        this->UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
          facePositionIndices, pIndicesOffset, originalData.Indices);
      }

      // Texture coordinate
      Alembic::AbcGeom::IV2fGeomParam uvsParam = schema.getUVsParam();
      if (uvsParam.valid())
      {
        Alembic::AbcGeom::IV2fGeomParam::Sample uvValue = uvsParam.getIndexedValue(selector);
        if (uvValue.valid())
        {
          V3fContainer uvV3F;
          Alembic::AbcGeom::UInt32ArraySamplePtr uvIndices = uvValue.getIndices();
          for (size_t index = 0; index < uvValue.getVals()->size(); ++index)
          {
            Alembic::AbcGeom::V2f uv = (*(uvValue.getVals()))[index];
            uvV3F.emplace_back(uv[0], uv[1], 0);
          }
          originalData.Attributes.insert(AttributesContainer::value_type("uv", uvV3F));

          if (uvsParam.getScope() == Alembic::AbcGeom::kFacevaryingScope)
          {
            originalData.uvFaceVarying = true;
            this->UpdateIndices<Alembic::AbcGeom::UInt32ArraySamplePtr>(
              uvIndices, uvIndicesOffset, originalData.Indices);
          }
          else
          {
            this->UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
              facePositionIndices, uvIndicesOffset, originalData.Indices);
          }
        }
      }

      // Normals
      Alembic::AbcGeom::IN3fGeomParam normalsParam = schema.getNormalsParam();
      if (normalsParam.valid())
      {
        Alembic::AbcGeom::IN3fGeomParam::Sample normalValue =
          normalsParam.getIndexedValue(selector);
        if (normalValue.valid())
        {
          V3fContainer normal_v3f;
          Alembic::AbcGeom::UInt32ArraySamplePtr normalIndices = normalValue.getIndices();
          for (size_t index = 0; index < normalValue.getVals()->size(); ++index)
          {
            Alembic::AbcGeom::V3f normal = (*(normalValue.getVals()))[index];
            normal_v3f.emplace_back(normal[0], normal[1], normal[2]);
          }
          originalData.Attributes.insert(AttributesContainer::value_type("N", normal_v3f));

          if (normalsParam.getScope() == Alembic::AbcGeom::kFacevaryingScope)
          {
            originalData.nFaceVarying = true;

            this->UpdateIndices<Alembic::AbcGeom::UInt32ArraySamplePtr>(
              normalIndices, nIndicesOffset, originalData.Indices);
          }
          else
          {
            this->UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
              facePositionIndices, nIndicesOffset, originalData.Indices);
          }
        }
      }
    }

    IntermediateGeometry duplicatedData;

    this->PointDuplicateAccumulator(originalData, duplicatedData);

    this->FillPolyData(duplicatedData, polydata);

    return polydata;
  }

  void ImportRoot(vtkAppendPolyData* append, double time)
  {
    const Alembic::Abc::IObject top = this->Archive.getTop();
    const Alembic::AbcGeom::ISampleSelector selector(time);
    Alembic::Abc::M44d identity;
    identity.makeIdentity();

    std::stack<std::tuple<const Alembic::Abc::IObject, const Alembic::Abc::ObjectHeader,
      const Alembic::Abc::M44d>>
      objects;

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      objects.emplace(std::make_tuple(top, top.getChildHeader(i), identity));
    }

    while (!objects.empty())
    {
      const auto& [parent, ohead, matrix] = objects.top();
      objects.pop();
      if (Alembic::AbcGeom::IPolyMesh::matches(ohead))
      {
        const Alembic::AbcGeom::IPolyMesh polymesh(parent, ohead.getName());
        append->AddInputData(ProcessIPolyMesh(polymesh, time, matrix));
      }
      else if (Alembic::AbcGeom::IXform::matches(ohead))
      {
        const Alembic::AbcGeom::IXform xForm(parent, ohead.getName());
        const Alembic::AbcGeom::IXformSchema& xFormSchema = xForm.getSchema();
        Alembic::AbcGeom::XformSample xFormSamp;
        xFormSchema.get(xFormSamp, selector);
        const Alembic::Abc::M44d xFormMatrix = matrix * xFormSamp.getMatrix();
        for (size_t i = 0; i < xForm.getNumChildren(); ++i)
        {
          objects.emplace(std::make_tuple(xForm, xForm.getChildHeader(i), xFormMatrix));
        }
      }
    }
  }

  void ExtendTimeRange(double& start, double& end)
  {
    Alembic::Abc::IObject top = this->Archive.getTop();

    std::stack<std::tuple<const Alembic::Abc::IObject, const Alembic::Abc::ObjectHeader>> objects;

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      objects.emplace(std::make_tuple(top, top.getChildHeader(i)));
    }

    while (!objects.empty())
    {
      auto [parent, ohead] = objects.top();
      objects.pop();
      int numSamples;
      Alembic::Abc::TimeSamplingPtr ts;
      if (Alembic::AbcGeom::IXform::matches(ohead))
      {
        const Alembic::AbcGeom::IXform xForm(parent, ohead.getName());
        const Alembic::AbcGeom::IXformSchema schema = xForm.getSchema();
        ts = schema.getTimeSampling();
        numSamples = schema.getNumSamples();
        for (size_t i = 0; i < xForm.getNumChildren(); ++i)
        {
          objects.emplace(std::make_tuple(xForm, xForm.getChildHeader(i)));
        }
      }
      else if (Alembic::AbcGeom::IPolyMesh::matches(ohead))
      {
        const Alembic::AbcGeom::IPolyMesh polymesh(parent, ohead.getName());
        const Alembic::AbcGeom::IPolyMeshSchema schema = polymesh.getSchema();
        ts = schema.getTimeSampling();
        numSamples = schema.getNumSamples();
      }

      if (ts->getTimeSamplingType().isUniform())
      {
        double min = ts->getSampleTime(0);
        double max = min + (numSamples - 1) * ts->getTimeSamplingType().getTimePerCycle();
        start = std::min(start, min);
        end = std::max(end, max);
      }
      else if (ts->getTimeSamplingType().isCyclic())
      {
        const auto& times = ts->getStoredTimes();
        start = std::min(start, times.front());
        end = std::max(end, times.back());
      }
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

//------------------------------------------------------------------------------
int vtkF3DAlembicReader::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  this->Internals->ReadArchive(this->FileName);

  double timeRange[2] = { std::numeric_limits<double>::infinity(),
    -std::numeric_limits<double>::infinity() };
  this->Internals->ExtendTimeRange(timeRange[0], timeRange[1]);

  if (timeRange[0] < timeRange[1])
  {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkF3DAlembicReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  double requestedTimeValue = 0.0;

  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
  {
    requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
  }

  vtkNew<vtkAppendPolyData> append;
  this->Internals->ImportRoot(append, requestedTimeValue);
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
