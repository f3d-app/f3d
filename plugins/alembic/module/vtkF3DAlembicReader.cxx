#include "vtkF3DAlembicReader.h"

#include <vtkAppendPolyData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkResourceStream.h>
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
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <numeric>
#include <stack>
#include <tuple>

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
  std::vector<vtkIdType> PointSourceIds;
  std::vector<vtkIdType> NormalSourceIds;
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

      size_t totalVertices = 0;
      for (size_t i = 0; i < faceCount; i++)
      {
        auto thisFaceVertexCount = originalData.Indices[i].size();
        duplicatedData.Indices[i].resize(thisFaceVertexCount, Alembic::Abc::V3i(0));
        totalVertices += thisFaceVertexCount;
      }

      duplicatedData.PointSourceIds.reserve(totalVertices);
      if (haveN)
      {
        duplicatedData.NormalSourceIds.reserve(totalVertices);
      }

      // Points
      {
        V3fContainer pV3F;
        pV3F.reserve(totalVertices);
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
            duplicatedData.PointSourceIds.emplace_back(originalData.Indices[i][j].x);
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
            duplicatedData.NormalSourceIds.push_back(originalData.Indices[i][j].z);
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

    const auto& pArray = pMapIter->second;
    const vtkIdType numPoints = static_cast<vtkIdType>(pArray.size());

    points->SetNumberOfPoints(numPoints);

    for (vtkIdType i = 0; i < numPoints; ++i)
    {
      const auto& p = pArray[i];
      points->SetPoint(i, p.x, p.y, p.z);
    }
    polydata->SetPoints(points);

    vtkIdType numCells = static_cast<vtkIdType>(data.Indices.size());
    vtkIdType totalConnectivitySize =
      std::accumulate(data.Indices.begin(), data.Indices.end(), vtkIdType(0),
        [](vtkIdType sum, const auto& face) { return sum + static_cast<vtkIdType>(face.size()); });

    vtkNew<vtkIdTypeArray> offsets;
    offsets->SetNumberOfTuples(numCells + 1);

    vtkNew<vtkIdTypeArray> connectivity;
    connectivity->SetNumberOfTuples(totalConnectivitySize);

    vtkIdType* offsetsPtr = offsets->GetPointer(0);
    vtkIdType* connPtr = connectivity->GetPointer(0);

    vtkIdType currentConnectivityIdx = 0;

    for (vtkIdType i = 0; i < numCells; i++)
    {
      offsetsPtr[i] = currentConnectivityIdx;
      const auto& face = data.Indices[i];
      for (const auto& triplet : face)
      {
        connPtr[currentConnectivityIdx++] = triplet.x;
      }
    }
    offsetsPtr[numCells] = currentConnectivityIdx;
    cells->SetData(offsets, connectivity);
    // Note : uv and N are optional
    auto uvMapIter = data.Attributes.find("uv");
    auto nMapIter = data.Attributes.find("N");
    bool haveUV = uvMapIter != data.Attributes.end();
    bool haveN = nMapIter != data.Attributes.end();

    polydata->SetPolys(cells);
    vtkDataSetAttributes* pointAttributes = polydata->GetAttributes(vtkDataSet::POINT);

    if (haveN)
    {
      vtkNew<vtkFloatArray> normals;
      normals->SetName("Normals");
      normals->SetNumberOfComponents(3);

      const auto& nArray = nMapIter->second;
      const vtkIdType numNormals = static_cast<vtkIdType>(nArray.size());
      normals->SetNumberOfTuples(numNormals);
      for (vtkIdType i = 0; i < numNormals; i++)
      {
        const auto& n = nArray[i];
        normals->SetTuple3(i, n.x, n.y, n.z);
      }
      pointAttributes->SetNormals(normals);
    }

    if (haveUV)
    {
      vtkNew<vtkFloatArray> uvs;
      uvs->SetName("UVs");
      uvs->SetNumberOfComponents(2);

      const auto& uvArray = uvMapIter->second;
      vtkIdType numUVs = static_cast<vtkIdType>(uvArray.size());
      uvs->SetNumberOfTuples(numUVs);

      for (vtkIdType i = 0; i < numUVs; i++)
      {
        const auto& uv = uvArray[i];
        uvs->SetTuple2(i, uv.x, uv.y);
      }

      pointAttributes->SetTCoords(uvs);
    }

    if (!data.PointSourceIds.empty())
    {
      vtkNew<vtkIdTypeArray> sourceIds;
      sourceIds->SetName("AbcSourceIds");
      sourceIds->SetNumberOfTuples(data.PointSourceIds.size());
      std::copy(data.PointSourceIds.begin(), data.PointSourceIds.end(), sourceIds->GetPointer(0));
      polydata->GetFieldData()->AddArray(sourceIds);
    }

    if (haveN && !data.NormalSourceIds.empty() &&
      data.NormalSourceIds.size() == static_cast<size_t>(numPoints))
    {
      vtkNew<vtkIdTypeArray> normalSourceIds;
      normalSourceIds->SetName("AbcNormalIds");
      normalSourceIds->SetNumberOfTuples(data.NormalSourceIds.size());
      std::copy(
        data.NormalSourceIds.begin(), data.NormalSourceIds.end(), normalSourceIds->GetPointer(0));
      polydata->GetFieldData()->AddArray(normalSourceIds);
    }
  }

public:
  std::map<std::string, vtkSmartPointer<vtkPolyData>> OutputCache;
  vtkSmartPointer<vtkPolyData> ProcessIPolyMesh(
    const Alembic::AbcGeom::IPolyMesh& pmesh, double time, const Alembic::Abc::M44d& matrix)
  {
    vtkNew<vtkPolyData> polydata;
    IntermediateGeometry originalData;

    Alembic::AbcGeom::IPolyMeshSchema::Sample samp;
    const Alembic::AbcGeom::IPolyMeshSchema& schema = pmesh.getSchema();

    if (schema.getNumSamples() == 0)
    {
      return polydata;
    }

    Alembic::AbcGeom::ISampleSelector selector(time);
    schema.get(samp, selector);
    const std::string& meshName = pmesh.getName();
    auto topologyVariance = schema.getTopologyVariance();
    bool isTopologyConstant = (topologyVariance == Alembic::AbcGeom::kConstantTopology) ||
      (topologyVariance == Alembic::AbcGeom::kHomogenousTopology);
    Alembic::AbcGeom::P3fArraySamplePtr positions = samp.getPositions();

    if (isTopologyConstant && this->OutputCache.count(meshName))
    {
      vtkPolyData* cachedPoly = this->OutputCache[meshName];
      polydata->ShallowCopy(cachedPoly);

      vtkDataArray* sourceIdsDA = polydata->GetFieldData()->GetArray("AbcSourceIds");

      const vtkIdType numPoints = polydata->GetNumberOfPoints();
      vtkNew<vtkPoints> newPoints;
      newPoints->SetNumberOfPoints(numPoints);
      if (sourceIdsDA)
      {
        vtkIdTypeArray* sourceIdsArr = vtkIdTypeArray::SafeDownCast(sourceIdsDA);
        if (sourceIdsArr)
        {
          const vtkIdType* srcIndices = sourceIdsArr->GetPointer(0);
          for (vtkIdType i = 0; i < numPoints; i++)
          {
            vtkIdType rawIndex = srcIndices[i];
            if (rawIndex < static_cast<vtkIdType>(positions->size()))
            {
              Alembic::Abc::V3f tp;
              matrix.multVecMatrix(positions->get()[rawIndex], tp);
              newPoints->SetPoint(i, tp.x, tp.y, tp.z);
            }
          }
        }
      }
      else
      {
        for (size_t i = 0; i < positions->size() && i < static_cast<size_t>(numPoints); i++)
        {
          const Alembic::Abc::V3f tp = positions->get()[i] * matrix;
          newPoints->SetPoint(i, tp.x, tp.y, tp.z);
        }
      }
      polydata->SetPoints(newPoints);

      // Update Normals
      vtkDataArray* normalSourceIdsDA = polydata->GetFieldData()->GetArray("AbcNormalIds");
      Alembic::AbcGeom::IN3fGeomParam normalsParam = schema.getNormalsParam();
      if (normalsParam.valid())
      {
        Alembic::AbcGeom::IN3fGeomParam::Sample normalValue =
          normalsParam.getIndexedValue(selector);
        if (normalValue.valid())
        {
          vtkFloatArray* normals =
            vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetNormals());
          if (normals)
          {
            const vtkIdType numNormals = normals->GetNumberOfTuples();
            vtkIdType* normalIndices = normalSourceIdsDA
              ? static_cast<vtkIdTypeArray*>(normalSourceIdsDA)->GetPointer(0)
              : nullptr;

            auto vals = normalValue.getVals();
            if (vals)
            {
              for (vtkIdType i = 0; i < numNormals; i++)
              {
                const vtkIdType rawIndex = normalIndices ? normalIndices[i] : i;
                if (rawIndex < static_cast<vtkIdType>(vals->size()))
                {
                  Alembic::Abc::V3f normal;
                  matrix.multDirMatrix((*vals)[rawIndex], normal);
                  normals->SetTuple3(i, normal.x, normal.y, normal.z);
                }
              }
            }
          }
        }
      }
    }
    else
    {
      Alembic::AbcGeom::Int32ArraySamplePtr facePositionIndices = samp.getFaceIndices();
      Alembic::AbcGeom::Int32ArraySamplePtr faceVertexCounts = samp.getFaceCounts();
      this->SetupIndicesStorage(faceVertexCounts, originalData.Indices);

      // By default, Alembic is CW while VTK is CCW
      // So we need to reverse the order of indices only if the mesh is not mirrored
      const bool doReverseRotate = matrix.determinant() > 0;

      // Positions
      {
        V3fContainer pV3F;
        for (size_t pIndex = 0; pIndex < positions->size(); pIndex++)
        {
          Alembic::Abc::V3f tp;
          matrix.multVecMatrix(positions->get()[pIndex], tp);
          pV3F.emplace_back(tp.x, tp.y, tp.z);
        }
        originalData.Attributes.insert(AttributesContainer::value_type("P", pV3F));
        this->UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
          facePositionIndices, pIndicesOffset, originalData.Indices, doReverseRotate);
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
              uvIndices, uvIndicesOffset, originalData.Indices, doReverseRotate);
          }
          else
          {
            this->UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
              facePositionIndices, uvIndicesOffset, originalData.Indices, doReverseRotate);
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
            Alembic::AbcGeom::V3f normal;
            matrix.multDirMatrix((*(normalValue.getVals()))[index], normal);
            normal_v3f.emplace_back(normal);
          }
          originalData.Attributes.insert(AttributesContainer::value_type("N", normal_v3f));
          if (normalsParam.getScope() == Alembic::AbcGeom::kFacevaryingScope)
          {
            originalData.nFaceVarying = true;
            this->UpdateIndices<Alembic::AbcGeom::UInt32ArraySamplePtr>(
              normalIndices, nIndicesOffset, originalData.Indices, doReverseRotate);
          }
          else
          {
            this->UpdateIndices<Alembic::AbcGeom::Int32ArraySamplePtr>(
              facePositionIndices, nIndicesOffset, originalData.Indices, doReverseRotate);
          }
        }
      }

      IntermediateGeometry duplicatedData;
      this->PointDuplicateAccumulator(originalData, duplicatedData);

      this->FillPolyData(duplicatedData, polydata);

      // Store data for the next frame
      if (isTopologyConstant)
      {
        this->OutputCache[meshName] = polydata;
      }
    }
    return polydata;
  }

  vtkSmartPointer<vtkPolyData> ProcessICurves(
    const Alembic::AbcGeom::ICurves& curve, double time, const Alembic::Abc::M44d& matrix)
  {
    vtkNew<vtkPolyData> polydata;

    const Alembic::AbcGeom::ICurvesSchema& schema = curve.getSchema();
    Alembic::AbcGeom::ICurvesSchema::Sample samp;

    if (schema.getNumSamples() > 0)
    {
      Alembic::AbcGeom::ISampleSelector selector(time);
      schema.get(samp, selector);

      Alembic::AbcGeom::P3fArraySamplePtr positions = samp.getPositions();
      Alembic::AbcGeom::Int32ArraySamplePtr curveCounts = samp.getCurvesNumVertices();

      vtkNew<vtkPoints> points;
      vtkNew<vtkCellArray> lines;

      for (size_t pIndex = 0; pIndex < positions->size(); ++pIndex)
      {
        Alembic::Abc::V3f tp;
        matrix.multVecMatrix(positions->get()[pIndex], tp);
        points->InsertNextPoint(tp.x, tp.y, tp.z);
      }

      size_t pOffsetIndex = 0;
      for (size_t cIndex = 0; cIndex < curveCounts->size(); ++cIndex)
      {
        const size_t vCount = curveCounts->get()[cIndex];

        vtkNew<vtkPolyLine> polyLine;
        polyLine->GetPointIds()->SetNumberOfIds(vCount);
        for (size_t j = 0; j < vCount; ++j)
        {
          polyLine->GetPointIds()->SetId(j, pOffsetIndex + j);
        }

        lines->InsertNextCell(polyLine);
        pOffsetIndex += vCount;
      }

      polydata->SetPoints(points);
      polydata->SetLines(lines);
    }
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
      const Alembic::AbcGeom::IObject obj(parent, ohead.getName());
      Alembic::Abc::M44d objMatrix = matrix;
      if (Alembic::AbcGeom::IPolyMesh::matches(ohead))
      {
        const Alembic::AbcGeom::IPolyMesh polymesh(parent, ohead.getName());
        append->AddInputData(this->ProcessIPolyMesh(polymesh, time, objMatrix));
      }
      else if (Alembic::AbcGeom::ICurves::matches(ohead))
      {
        const Alembic::AbcGeom::ICurves curve(parent, ohead.getName());
        append->AddInputData(this->ProcessICurves(curve, time, objMatrix));
      }
      else if (Alembic::AbcGeom::IXform::matches(ohead))
      {
        const Alembic::AbcGeom::IXform xForm(parent, ohead.getName());
        const Alembic::AbcGeom::IXformSchema& xFormSchema = xForm.getSchema();
        Alembic::AbcGeom::XformSample xFormSamp;
        xFormSchema.get(xFormSamp, selector);
        objMatrix = xFormSamp.getMatrix() * matrix;
      }

      objects.pop();
      for (size_t i = 0; i < obj.getNumChildren(); ++i)
      {
        objects.emplace(std::make_tuple(obj, obj.getChildHeader(i), objMatrix));
      }
    }
  }

  void FetchTimeSteps(std::vector<double>& timeSteps)
  {
    Alembic::Abc::IObject top = this->Archive.getTop();

    std::set<double> timeStepSet;
    std::stack<std::pair<const Alembic::Abc::IObject, const Alembic::Abc::ObjectHeader>> objects;

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      objects.emplace(std::make_pair(top, top.getChildHeader(i)));
    }

    while (!objects.empty())
    {
      const auto& [parent, ohead] = objects.top();
      const Alembic::AbcGeom::IObject obj(parent, ohead.getName());
      int numSamples = 0;
      Alembic::Abc::TimeSamplingPtr ts;
      if (Alembic::AbcGeom::IXform::matches(ohead))
      {
        const Alembic::AbcGeom::IXform xForm(parent, ohead.getName());
        const Alembic::AbcGeom::IXformSchema& schema = xForm.getSchema();
        ts = schema.getTimeSampling();
        numSamples = static_cast<int>(schema.getNumSamples());
      }
      else if (Alembic::AbcGeom::IPolyMesh::matches(ohead))
      {
        const Alembic::AbcGeom::IPolyMesh polymesh(parent, ohead.getName());
        const Alembic::AbcGeom::IPolyMeshSchema& schema = polymesh.getSchema();
        ts = schema.getTimeSampling();
        numSamples = static_cast<int>(schema.getNumSamples());
      }
      else if (Alembic::AbcGeom::ICurves::matches(ohead))
      {
        const Alembic::AbcGeom::ICurves curves(parent, ohead.getName());
        const Alembic::AbcGeom::ICurvesSchema& schema = curves.getSchema();
        ts = schema.getTimeSampling();
        numSamples = static_cast<int>(schema.getNumSamples());
      }

      objects.pop();
      for (size_t i = 0; i < obj.getNumChildren(); ++i)
      {
        objects.emplace(std::make_pair(obj, obj.getChildHeader(i)));
      }

      if (ts == nullptr)
      {
        continue;
      }

      const auto& times = ts->getStoredTimes();
      for (auto& timeStep : times)
      {
        timeStepSet.insert(timeStep);
      }
    }
    timeSteps = std::vector<double>(timeStepSet.begin(), timeStepSet.end());
  }

  void ComputeTimeRangeAndSteps(double& start, double& end, std::vector<double>& timeSteps)
  {
    Alembic::Abc::IObject top = this->Archive.getTop();

    // Using std::set since we need time steps to be unique and ordered
    std::set<double> timeStepSet;
    std::stack<std::pair<const Alembic::Abc::IObject, const Alembic::Abc::ObjectHeader>> objects;

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      objects.emplace(std::make_pair(top, top.getChildHeader(i)));
    }

    while (!objects.empty())
    {
      const auto& [parent, ohead] = objects.top();
      const Alembic::AbcGeom::IObject obj(parent, ohead.getName());
      int numSamples = 0;
      Alembic::Abc::TimeSamplingPtr ts;
      if (Alembic::AbcGeom::IXform::matches(ohead))
      {
        const Alembic::AbcGeom::IXform xForm(parent, ohead.getName());
        const Alembic::AbcGeom::IXformSchema& schema = xForm.getSchema();
        ts = schema.getTimeSampling();
        numSamples = static_cast<int>(schema.getNumSamples());
      }
      else if (Alembic::AbcGeom::IPolyMesh::matches(ohead))
      {
        const Alembic::AbcGeom::IPolyMesh polymesh(parent, ohead.getName());
        const Alembic::AbcGeom::IPolyMeshSchema& schema = polymesh.getSchema();
        ts = schema.getTimeSampling();
        numSamples = static_cast<int>(schema.getNumSamples());
      }
      else if (Alembic::AbcGeom::ICurves::matches(ohead))
      {
        const Alembic::AbcGeom::ICurves curves(parent, ohead.getName());
        const Alembic::AbcGeom::ICurvesSchema& schema = curves.getSchema();
        ts = schema.getTimeSampling();
        numSamples = static_cast<int>(schema.getNumSamples());
      }

      objects.pop();
      for (size_t i = 0; i < obj.getNumChildren(); ++i)
      {
        objects.emplace(std::make_pair(obj, obj.getChildHeader(i)));
      }

      if (ts == nullptr)
      {
        continue;
      }

      // Collecting all time steps
      const auto& times = ts->getStoredTimes();
      for (auto& timeStep : times)
      {
        timeStepSet.insert(timeStep);
      }
    }
    start = *timeStepSet.begin();
    end = *timeStepSet.rbegin();
    timeSteps = std::vector<double>(timeStepSet.begin(), timeStepSet.end());
  }

  bool ReadArchive(
    vtkResourceStream* stream, const std::string& filePath, [[maybe_unused]] vtkObject* parent)
  {
    Alembic::AbcCoreFactory::IFactory factory;
    Alembic::AbcCoreFactory::IFactory::CoreType coreType;

    if (stream)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251210)
      // Encapsulate resource stream into an istream
      stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
      this->Streambuf = stream->ToStreambuf();
      this->Buffer = std::make_unique<std::istream>(this->Streambuf.get());
      try
      {
        this->Archive =
          factory.getArchive(std::vector<std::istream*>({ this->Buffer.get() }), coreType);
      }
      catch (Alembic::Util::v12::Exception& ex)
      {
        vtkErrorWithObjectMacro(parent, "Error reading stream: " << ex.what());
        return false;
      }
#else
      vtkErrorWithObjectMacro(
        parent, "This version of VTK doesn't support reading memory stream with Alembic");
      return false;
#endif
    }
    else
    {
      this->Archive = factory.getArchive(filePath, coreType);
    }
    return this->Archive.valid();
  }
  Alembic::Abc::IArchive Archive;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251210)
  std::unique_ptr<std::streambuf> Streambuf;
  std::unique_ptr<std::istream> Buffer;
#endif
};

vtkStandardNewMacro(vtkF3DAlembicReader);
vtkCxxSetSmartPointerMacro(vtkF3DAlembicReader, Stream, vtkResourceStream);

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
  if (!this->Internals->ReadArchive(this->Stream, this->FileName, this))
  {
    vtkErrorMacro("Unable to read this alembic file or stream");
    return 0;
  }

  double timeRange[2] = { std::numeric_limits<double>::infinity(),
    -std::numeric_limits<double>::infinity() };
  std::vector<double> timeSteps {0};
  this->Internals->ComputeTimeRangeAndSteps(timeRange[0], timeRange[1], timeSteps);

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (timeRange[0] < timeRange[1])
  {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  }
  if (timeSteps.size() > 0)
  {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), timeSteps.data(), timeSteps.size());
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
vtkMTimeType vtkF3DAlembicReader::GetMTime()
{
  auto mtime = this->Superclass::GetMTime();
  if (this->Stream)
  {
    mtime = std::max(mtime, this->Stream->GetMTime());
  }
  return mtime;
}
//------------------------------------------------------------------------------
bool vtkF3DAlembicReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  constexpr std::string_view abcMagic{ "Ogawa", 5 };

  std::array<char, 5> magic;
  if (stream->Read(&magic, magic.size()) != magic.size())
  {
    return false;
  }

  return std::string_view(magic.data(), magic.size()) == abcMagic;
}
