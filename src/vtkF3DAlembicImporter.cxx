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

#include <vtksys/SystemTools.hxx>

#include <Alembic/AbcGeom/All.h>
// #include <Alembic/AbcCoreHDF5/All.h> Not sure if we want to support legacy HDF5 stuff
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/Util/All.h>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreFactory/All.h>

vtkStandardNewMacro(vtkF3DAlembicImporter);


class vtkF3DAlembicImporterInternalEx
{
	typedef std::map<Alembic::AbcGeom::PlainOldDataType,std::string> PODStringMap;
	typedef std::map<Alembic::AbcGeom::PropertyType,std::string> PropertyTypeStringMap;
public:
	vtkF3DAlembicImporterInternalEx(vtkF3DAlembicImporter* parent) {
		this->Parent = parent;
	}
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
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
		vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

		Alembic::AbcGeom::IPolyMeshSchema::Sample samp;
		if ( pmesh.getSchema().getNumSamples() > 0 )
		{
			pmesh.getSchema().get( samp );

			Alembic::AbcGeom::P3fArraySamplePtr P = samp.getPositions();
			Alembic::AbcGeom::Int32ArraySamplePtr indices = samp.getFaceIndices();
			Alembic::AbcGeom::Int32ArraySamplePtr counts = samp.getFaceCounts();
			Alembic::AbcGeom::V3fArraySamplePtr v = samp.getVelocities();

			size_t P_size = P->size();
			size_t indices_size = indices->size();
			size_t counts_size = counts->size();

			std::cout << "P size = " << P_size << std::endl;
			std::cout << "indices size = " << indices_size << std::endl;
			std::cout << "counts size = " << counts_size << std::endl;
			if (v != 0)
			{
				size_t v_size = v->size();
				std::cout << "v size = " << v_size << std::endl;
			}

			for (size_t i=0;i<P_size;i++)
			{
				points->InsertNextPoint(P->get()[i].x, P->get()[i].y, P->get()[i].z);
			}

			size_t face_index = 0;
			for (size_t i=0;i<counts_size;i++)
			{
				size_t polyface_vertex_count = counts->get()[i];
				polys->InsertNextCell(polyface_vertex_count);
				for (size_t j=0;j<polyface_vertex_count;j++)
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

	void IterateIObject(vtkRenderer* renderer,
						const Alembic::Abc::IObject &parent,
						const Alembic::Abc::ObjectHeader &ohead)
	{
		std::cout << "Parent Full name " << parent.getFullName() << std::endl;
		std::cout << "ohead Full name " << ohead.getFullName() << std::endl;

		//set this if we should continue traversing
		Alembic::Abc::IObject nextParentObject;

		if ( Alembic::AbcGeom::IXform::matches( ohead ) )
		{
			std::cout << "iterate_iobject match IXform" << std::endl;
			Alembic::AbcGeom::IXform xform( parent, ohead.getName() );

			nextParentObject = xform;

		}
		else if ( Alembic::AbcGeom::ISubD::matches( ohead ) )
		{
			std::cout << "iterate_iobject match ISubD" << std::endl;
			Alembic::AbcGeom::ISubD subd( parent, ohead.getName() );
			nextParentObject = subd;

		}
		else if ( Alembic::AbcGeom::IPolyMesh::matches( ohead ) )
		{
			std::cout << "iterate_iobject match IPolyMesh" << std::endl;
			Alembic::AbcGeom::IPolyMesh polymesh( parent, ohead.getName() );
			ProcessIPolyMesh(renderer, polymesh);
			nextParentObject = polymesh;

		}
		else if ( Alembic::AbcGeom::INuPatch::matches( ohead ) )
		{
			std::cout << "iterate_iobject match INuPatch" << std::endl;

		}
		else if ( Alembic::AbcGeom::IPoints::matches( ohead ) )
		{
			std::cout << "iterate_iobject match IPoints" << std::endl;
			Alembic::AbcGeom::IPoints points( parent, ohead.getName() );
			// ProcessIPoints(points);
			nextParentObject = points;
		}
		else if ( Alembic::AbcGeom::ICurves::matches( ohead ) )
		{
			std::cout << "iterate_iobject match ICurves" << std::endl;

		}
		else if ( Alembic::AbcGeom::IFaceSet::matches( ohead ) )
		{
			std::cout << "iterate_iobject match IFaceSet" << std::endl;
			std::cerr << "DOH !" << std::endl;
		}

		// Recursion
		if ( nextParentObject.valid() )
		{
			for ( size_t i = 0; i < nextParentObject.getNumChildren() ; ++i )
			{
				IterateIObject( renderer, nextParentObject, nextParentObject.getChildHeader( i ));
			}
		}

	}
	void ImportRoot(vtkRenderer* renderer)
	{
		Alembic::Abc::IObject top = _archive.getTop();
		size_t top_num_children = top.getNumChildren();

		//   vtkDebugMacro("Done with " << this->GetClassName() << "::" << __FUNCTION__);
		std::cout <<  top_num_children << std::endl;

	    for ( size_t i = 0; i < top.getNumChildren(); ++i )
	    {
	    	IterateIObject(renderer, top, top.getChildHeader(i));
	    }

	}
	void ImportCameras(vtkRenderer* renderer)
	{

	}
	void ReadScene(const std::string& filePath)
	{

		Alembic::AbcCoreFactory::IFactory factory;
		Alembic::AbcCoreFactory::IFactory::CoreType core_type;

		_archive = factory.getArchive(filePath, core_type);

	}
	std::string Description;
	std::vector<vtkSmartPointer<vtkPolyData> > Meshes;
	std::vector<vtkSmartPointer<vtkProperty> > Properties;
	std::vector<vtkSmartPointer<vtkTexture> > EmbeddedTextures;
	vtkIdType ActiveAnimation = 0;
	std::vector<std::pair<std::string, vtkSmartPointer<vtkLight> > > Lights;
	std::vector<std::pair<std::string, vtkSmartPointer<vtkCamera> > > Cameras;
	vtkIdType ActiveCameraIndex = -1;
	std::unordered_map<std::string, vtkSmartPointer<vtkActorCollection> > NodeActors;
	std::unordered_map<std::string, vtkSmartPointer<vtkMatrix4x4> > NodeLocalMatrix;
	std::unordered_map<std::string, vtkSmartPointer<vtkMatrix4x4> > NodeTRSMatrix;
	std::unordered_map<std::string, vtkSmartPointer<vtkMatrix4x4> > NodeGlobalMatrix;
	vtkF3DAlembicImporter* Parent;
	//
	Alembic::Abc::IArchive _archive;

};


//----------------------------------------------------------------------------
vtkF3DAlembicImporter::vtkF3DAlembicImporter()
{
	this->InternalsEx = new vtkF3DAlembicImporterInternalEx(this);
}

//----------------------------------------------------------------------------
vtkF3DAlembicImporter::~vtkF3DAlembicImporter()
{
	delete this->InternalsEx;
}

//----------------------------------------------------------------------------
int vtkF3DAlembicImporter::ImportBegin()
{
	this->InternalsEx->ReadScene(this->FileName);

	std::cout << "vtkF3DAlembicImporter::ImportBegin()" << std::endl;
	return 1;
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::ImportActors(vtkRenderer* renderer)
{
	this->InternalsEx->ImportRoot(renderer);
}

//----------------------------------------------------------------------------
std::string vtkF3DAlembicImporter::GetOutputsDescription()
{
	return "";
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::UpdateTimeStep(double timestep)
{
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DAlembicImporter::GetNumberOfAnimations()
{
	return 0;
}

//----------------------------------------------------------------------------
std::string vtkF3DAlembicImporter::GetAnimationName(vtkIdType animationIndex)
{
	return "";
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::EnableAnimation(vtkIdType animationIndex)
{
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::DisableAnimation(vtkIdType vtkNotUsed(animationIndex))
{
}

//----------------------------------------------------------------------------
bool vtkF3DAlembicImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
	return false;
}

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200912)
//----------------------------------------------------------------------------
bool vtkF3DAlembicImporter::GetTemporalInformation(vtkIdType animationIndex, double frameRate,
												   int& nbTimeSteps, double timeRange[2], vtkDoubleArray* timeSteps)
{
	return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DAlembicImporter::GetNumberOfCameras()
{
	return 0;
}

//----------------------------------------------------------------------------
std::string vtkF3DAlembicImporter::GetCameraName(vtkIdType camIndex)
{
	return "";
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::SetCamera(vtkIdType camIndex)
{
}
#endif // VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200912)

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::ImportCameras(vtkRenderer* renderer)
{
	this->InternalsEx->ImportCameras(renderer);
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::ImportLights(vtkRenderer* renderer)
{
	// Alembic has no concept of lights
}

//----------------------------------------------------------------------------
void vtkF3DAlembicImporter::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "FileName: " << this->FileName << "\n";
}
