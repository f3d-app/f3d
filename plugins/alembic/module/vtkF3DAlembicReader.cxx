#include "vtkF3DAlembicReader.h"

#include <vtkAppendPolyData.h>
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

      size_t P_size = positions->size();
      size_t counts_size = counts->size();

      for (size_t i = 0; i < P_size; i++)
      {
        points->InsertNextPoint(
          positions->get()[i].x, positions->get()[i].y, positions->get()[i].z);
      }

      size_t face_index = 0;
      for (size_t i = 0; i < counts_size; i++)
      {
        auto polyface_vertex_count = counts->get()[i];
        polys->InsertNextCell(polyface_vertex_count);
        for (auto j = 0; j < polyface_vertex_count; j++)
        {
          polys->InsertCellPoint(indices->get()[face_index++]);
        }
      }
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
        IterateIObject(append, nextParentObject, nextParentObject.getChildHeader(i));
      }
    }
  }

  void ImportRoot(vtkAppendPolyData* append)
  {
    Alembic::Abc::IObject top = this->Archive.getTop();

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      IterateIObject(append, top, top.getChildHeader(i));
    }
  }

  void ReadScene(const std::string& filePath)
  {
    Alembic::AbcCoreFactory::IFactory factory;
    Alembic::AbcCoreFactory::IFactory::CoreType core_type;

    this->Archive = factory.getArchive(filePath, core_type);
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

  this->Internals->ReadScene(this->FileName);

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
