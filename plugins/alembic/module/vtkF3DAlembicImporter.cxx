#include "vtkF3DAlembicImporter.h"

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCamera.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkLight.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkQuaternion.h>
#include <vtkRenderer.h>
#include <vtkShaderProperty.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTexture.h>
#include <vtkTransform.h>
#include <vtkUniforms.h>
#include <vtkUnsignedShortArray.h>

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

class vtkF3DAlembicImporter::vtkInternals
{
  typedef std::map<Alembic::AbcGeom::PlainOldDataType, std::string> PODStringMap;
  typedef std::map<Alembic::AbcGeom::PropertyType, std::string> PropertyTypeStringMap;

public:
  void CreatePODStringMap(PODStringMap& podStringMap)
  {
    podStringMap[Alembic::AbcGeom::kBooleanPOD] = "kBooleanPOD";
    podStringMap[Alembic::AbcGeom::kUint8POD] = "kUint8POD";
    podStringMap[Alembic::AbcGeom::kInt8POD] = "kInt8POD";
    podStringMap[Alembic::AbcGeom::kUint16POD] = "kUint16POD";
    podStringMap[Alembic::AbcGeom::kInt16POD] = "kInt16POD";
    podStringMap[Alembic::AbcGeom::kUint32POD] = "kUint32POD";
    podStringMap[Alembic::AbcGeom::kInt32POD] = "kInt32POD";
    podStringMap[Alembic::AbcGeom::kUint64POD] = "kUint64POD";
    podStringMap[Alembic::AbcGeom::kInt64POD] = "kInt64POD";
    podStringMap[Alembic::AbcGeom::kFloat16POD] = "kFloat16POD";
    podStringMap[Alembic::AbcGeom::kFloat32POD] = "kFloat32POD";
    podStringMap[Alembic::AbcGeom::kFloat64POD] = "kFloat64POD";
    podStringMap[Alembic::AbcGeom::kStringPOD] = "kStringPOD";
    podStringMap[Alembic::AbcGeom::kWstringPOD] = "kWstringPOD";
    podStringMap[Alembic::AbcGeom::kNumPlainOldDataTypes] = "kNumPlainOldDataTypes";
    podStringMap[Alembic::AbcGeom::kUnknownPOD] = "kUnknownPOD";
  }

  void CreatePropertyTypeStringMap(PropertyTypeStringMap& propertyTypeStringMap)
  {
    propertyTypeStringMap[Alembic::AbcGeom::kCompoundProperty] = "kCompoundProperty";
    propertyTypeStringMap[Alembic::AbcGeom::kScalarProperty] = "kScalarProperty";
    propertyTypeStringMap[Alembic::AbcGeom::kArrayProperty] = "kArrayProperty";
  }

  void ProcessIPolyMesh(vtkRenderer* renderer, const Alembic::AbcGeom::IPolyMesh& pmesh)
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

    vtkNew<vtkPolyDataMapper> polyMapper;
    polyMapper->SetInputData(polydata);
    vtkNew<vtkActor> polyActor;
    polyActor->SetMapper(polyMapper);
    renderer->AddActor(polyActor);
  }

  void IterateIObject(vtkRenderer* renderer, const Alembic::Abc::IObject& parent,
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
      ProcessIPolyMesh(renderer, polymesh);
      nextParentObject = polymesh;
    }

    // Recursion
    if (nextParentObject.valid())
    {
      for (size_t i = 0; i < nextParentObject.getNumChildren(); ++i)
      {
        IterateIObject(renderer, nextParentObject, nextParentObject.getChildHeader(i));
      }
    }
  }

  void ImportRoot(vtkRenderer* renderer)
  {
    Alembic::Abc::IObject top = Archive.getTop();

    for (size_t i = 0; i < top.getNumChildren(); ++i)
    {
      IterateIObject(renderer, top, top.getChildHeader(i));
    }
  }

  void ImportCameras(vtkRenderer* vtkNotUsed(renderer)) {}

  void ImportLights(vtkRenderer* vtkNotUsed(renderer)) {}

  void ReadScene(const std::string& filePath)
  {

    Alembic::AbcCoreFactory::IFactory factory;
    Alembic::AbcCoreFactory::IFactory::CoreType core_type;

    Archive = factory.getArchive(filePath, core_type);
  }
  Alembic::Abc::IArchive Archive;
};

vtkStandardNewMacro(vtkF3DAlembicImporter);

//----------------------------------------------------------------------------
vtkF3DAlembicImporter::vtkF3DAlembicImporter()
  : Internals(new vtkF3DAlembicImporter::vtkInternals())
{
}

//----------------------------------------------------------------------------
vtkF3DAlembicImporter::~vtkF3DAlembicImporter() = default;

//----------------------------------------------------------------------------
int vtkF3DAlembicImporter::ImportBegin()
{
  this->Internals->ReadScene(this->FileName);

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::ImportActors(vtkRenderer* renderer)
{
  this->Internals->ImportRoot(renderer);
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
