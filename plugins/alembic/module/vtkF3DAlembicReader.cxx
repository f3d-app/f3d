#include "vtkF3DAlembicReader.h"

#include <vtkAppendPolyData.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
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

class vtkF3DAlembicReader::vtkInternals
{
public:
  vtkSmartPointer<vtkPolyData> ProcessIPolyMesh(const Alembic::AbcGeom::IPolyMesh& pmesh)
  {
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> polys;
    vtkNew<vtkPolyData> polydata;

    Alembic::AbcGeom::IPolyMeshSchema::Sample samp;
    if (pmesh.getSchema().getNumSamples() > 0)
    {
      pmesh.getSchema().get(samp);

      Alembic::AbcGeom::P3fArraySamplePtr positions = samp.getPositions();
      Alembic::AbcGeom::Int32ArraySamplePtr indices = samp.getFaceIndices();
      Alembic::AbcGeom::Int32ArraySamplePtr counts = samp.getFaceCounts();

      points->SetNumberOfPoints(positions->size());

      for (size_t i = 0; i < positions->size(); i++)
      {
        points->SetPoint(i, positions->get()[i].x, positions->get()[i].y, positions->get()[i].z);
      }

      vtkNew<vtkIdTypeArray> offsets;
      vtkNew<vtkIdTypeArray> connectivity;
      offsets->SetNumberOfTuples(counts->size() + 1);
      connectivity->SetNumberOfTuples(indices->size());

      offsets->SetTypedComponent(0, 0, 0);
      for (size_t i = 0; i < counts->size(); i++)
      {
        offsets->SetTypedComponent(i + 1, 0, offsets->GetTypedComponent(i, 0) + counts->get()[i]);
      }

      for (size_t i = 0; i < indices->size(); i++)
      {
        connectivity->SetTypedComponent(i, 0, indices->get()[i]);
      }

      polys->SetData(offsets, connectivity);
    }
    polydata->SetPoints(points);
    polydata->SetPolys(polys);

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
