#include "vtkF3DGLTFImporter.h"

#include "vtkF3DGLTFDocumentLoader.h"
#include "vtkF3DImporter.h"

#include <vtkActor.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

// need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13116
// which is backported in 9.6.2 in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13185
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 2)
#include <vtkActorCollection.h>
#include <vtkProperty.h>

#include <cmath>
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFImporter);

//----------------------------------------------------------------------------
vtkF3DGLTFImporter::vtkF3DGLTFImporter()
{
  this->SetImportArmature(true);
}

//----------------------------------------------------------------------------
void vtkF3DGLTFImporter::InitializeLoader()
{
  this->Loader = vtkSmartPointer<vtkF3DGLTFDocumentLoader>::New();
}

//----------------------------------------------------------------------------
void vtkF3DGLTFImporter::ApplyArmatureProperties(vtkActor* actor)
{
  this->Superclass::ApplyArmatureProperties(actor);

  vtkNew<vtkInformation> info;
  info->Set(vtkF3DImporter::ACTOR_IS_ARMATURE(), 1);
  actor->SetPropertyKeys(info);
}

// need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13116
// which is backported in 9.6.2 in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13185
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 2)
//----------------------------------------------------------------------------
void vtkF3DGLTFImporter::ImportActors(vtkRenderer* renderer)
{
  this->Superclass::ImportActors(renderer);

  // loop on actors
  vtkCollectionSimpleIterator ait;
  this->ActorCollection->InitTraversal(ait);
  while (vtkActor* actor = this->ActorCollection->GetNextActor(ait))
  {
    vtkProperty* prop = actor->GetProperty();
    if (prop->GetLighting() == false)
    {
      double color[3];
      prop->GetColor(color);

      // convert to linear space
      auto toLinear = [](double c) { return std::pow(c, 2.2); };
      color[0] = toLinear(color[0]);
      color[1] = toLinear(color[1]);
      color[2] = toLinear(color[2]);
      prop->SetColor(color);
    }
  }
}
#endif

//------------------------------------------------------------------------------
int vtkF3DGLTFImporter::ImportBegin()
{
  const int ret = this->Superclass::ImportBegin();

  if (ret != 0)
  {
    auto model = this->Loader->GetInternalModel();

    for (auto& mesh : model->Meshes)
    {
      for (auto& primitive : mesh.Primitives)
      {
        vtkPolyData* polyData = primitive.Geometry;
        vtkPointData* pointData = polyData->GetPointData();

        // Convert GLTF spherical harmonics float arrays to normalized unsigned char arrays
        // format expected by F3D point splat mapper.
        auto convertSH = [&](const std::string& floatArrayName, const std::string& ucharArrayName)
        {
          const vtkFloatArray* floatArray =
            vtkFloatArray::SafeDownCast(pointData->GetArray(floatArrayName.c_str()));
          if (floatArray)
          {
            vtkNew<vtkUnsignedCharArray> ucharArray;
            ucharArray->SetName(ucharArrayName.c_str());
            ucharArray->SetNumberOfComponents(3);
            ucharArray->SetNumberOfTuples(floatArray->GetNumberOfTuples());
            pointData->AddArray(ucharArray);

            // convert floats to normalized uchars
            for (vtkIdType i = 0; i < floatArray->GetNumberOfTuples(); ++i)
            {
              float tuple[3];
              floatArray->GetTypedTuple(i, tuple);

              unsigned char ucharTuple[3];
              for (int j = 0; j < 3; ++j)
              {
                ucharTuple[j] = static_cast<unsigned char>(
                  std::clamp((tuple[j] + 1.f) * 0.5f, 0.0f, 1.0f) * 255.0);
              }
              ucharArray->SetTypedTuple(i, ucharTuple);
            }
          }
        };

        convertSH("KHR_gaussian_splatting:SH_DEGREE_1_COEF_0", "sh1m1");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_1_COEF_1", "sh10");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_1_COEF_2", "sh1p1");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_2_COEF_0", "sh2m2");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_2_COEF_1", "sh2m1");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_2_COEF_2", "sh20");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_2_COEF_3", "sh2p1");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_2_COEF_4", "sh2p2");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_0", "sh3m3");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_1", "sh3m2");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_2", "sh3m1");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_3", "sh30");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_4", "sh3p1");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_5", "sh3p2");
        convertSH("KHR_gaussian_splatting:SH_DEGREE_3_COEF_6", "sh3p3");
      }
    }
  }

  return ret;
}

//------------------------------------------------------------------------------
bool vtkF3DGLTFImporter::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  vtkNew<vtkF3DGLTFDocumentLoader> loader;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260212)
  if (!loader->LoadModelMetaDataFromStream(stream, nullptr, true))
#else
  if (!loader->LoadModelMetaDataFromStream(stream))
#endif
  {
    return false;
  }
  return true;
}
