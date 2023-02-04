#include "vtkF3DAssimpImporter.h"

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

#include <assimp/Exceptional.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <memory>

vtkStandardNewMacro(vtkF3DAssimpImporter);

class vtkF3DAssimpImporter::vtkInternals
{
public:
  //----------------------------------------------------------------------------
  vtkInternals(vtkF3DAssimpImporter* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  /**
   * Import cameras
   */
  void ImportCameras(vtkRenderer* renderer)
  {
    if (this->Scene)
    {
      for (unsigned int i = 0; i < this->Scene->mNumCameras; i++)
      {
        aiCamera* aCam = this->Scene->mCameras[i];
        vtkNew<vtkCamera> vCam;
        vCam->SetPosition(aCam->mPosition[0], aCam->mPosition[1], aCam->mPosition[2]);
        vCam->SetViewUp(aCam->mUp[0], aCam->mUp[1], aCam->mUp[2]);
        vCam->SetFocalPoint(aCam->mPosition[0] + aCam->mLookAt[0],
          aCam->mPosition[1] + aCam->mLookAt[1], aCam->mPosition[2] + aCam->mLookAt[2]);
        this->Cameras.push_back({ aCam->mName.data, vCam });
      }

      // update camera global matrix nodes
      this->UpdateCameras();

      if (this->ActiveCameraIndex >= 0 &&
        this->ActiveCameraIndex < static_cast<vtkIdType>(this->Cameras.size()))
      {
        renderer->SetActiveCamera(this->Cameras[this->ActiveCameraIndex].second);
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Import lights
   */
  void ImportLights(vtkRenderer* renderer)
  {
    if (this->Scene)
    {
      int nbLights = this->Scene->mNumLights;

      if (nbLights > 0)
      {
        renderer->AutomaticLightCreationOff();

        for (int i = 0; i < nbLights; i++)
        {
          aiLight* aLight = this->Scene->mLights[i];

          vtkNew<vtkLight> light;
          light->SetPosition(aLight->mPosition[0], aLight->mPosition[1], aLight->mPosition[2]);
          light->SetFocalPoint(aLight->mPosition[0] + aLight->mDirection[0],
            aLight->mPosition[1] + aLight->mDirection[1],
            aLight->mPosition[2] + aLight->mDirection[2]);
          light->SetAttenuationValues(aLight->mAttenuationConstant, aLight->mAttenuationLinear,
            aLight->mAttenuationQuadratic);
          light->SetAmbientColor(
            aLight->mColorAmbient[0], aLight->mColorAmbient[1], aLight->mColorAmbient[2]);
          light->SetDiffuseColor(
            aLight->mColorDiffuse[0], aLight->mColorDiffuse[1], aLight->mColorDiffuse[2]);
          light->SetSpecularColor(
            aLight->mColorSpecular[0], aLight->mColorSpecular[1], aLight->mColorSpecular[2]);
          light->SetLightTypeToSceneLight();

          switch (aLight->mType)
          {
            case aiLightSourceType::aiLightSource_POINT:
              light->PositionalOn();
              break;
            case aiLightSourceType::aiLightSource_DIRECTIONAL:
              light->PositionalOff();
              break;
            case aiLightSourceType::aiLightSource_AMBIENT:
              light->PositionalOff();
              break;
            case aiLightSourceType::aiLightSource_SPOT:
              light->SetConeAngle(vtkMath::DegreesFromRadians(aLight->mAngleInnerCone));
              light->PositionalOn();
              break;
            case aiLightSourceType::aiLightSource_AREA:
            default:
              vtkWarningWithObjectMacro(
                this->Parent, "Unsupported light type: " << aLight->mName.data);
              break;
          }

          renderer->AddLight(light);

          this->Lights.push_back({ aLight->mName.data, light });
        }
      }

      // update light global matrix nodes
      this->UpdateLights();
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Generate a VTK texture from a file path
   */
  vtkSmartPointer<vtkTexture> CreateTexture(const char* path, bool sRGB = false)
  {
    vtkSmartPointer<vtkTexture> vTexture;

    if (path[0] == '*')
    {
      int texIndex = std::atoi(path + 1);
      vTexture = this->EmbeddedTextures[texIndex];
    }
    else
    {
      // sometimes, embedded textures are indexed by filename
      const aiTexture* aTexture = this->Scene->GetEmbeddedTexture(path);

      if (aTexture)
      {
        vTexture = this->CreateEmbeddedTexture(aTexture);
      }
      else
      {
        std::string dir = vtksys::SystemTools::GetParentDirectory(this->Parent->GetFileName());
        std::string texturePath = vtksys::SystemTools::CollapseFullPath(path, dir);

        // try to get the texture in the same dir as the model file
        if (!vtksys::SystemTools::FileExists(texturePath))
        {
          std::string fileName = vtksys::SystemTools::GetFilenameName(path);
          texturePath = vtksys::SystemTools::CollapseFullPath(fileName, dir);
        }

        if (vtksys::SystemTools::FileExists(texturePath))
        {
          vtkSmartPointer<vtkImageReader2> reader;
          reader.TakeReference(vtkImageReader2Factory::CreateImageReader2(texturePath.c_str()));

          if (!reader)
          {
            vtkWarningWithObjectMacro(
              this->Parent, "Cannot instantiate the image reader for texture: " << texturePath);
            return nullptr;
          }

          reader->SetFileName(texturePath.c_str());

          vTexture = vtkSmartPointer<vtkTexture>::New();
          vTexture->SetInputConnection(reader->GetOutputPort());
        }
        else
        {
          vtkWarningWithObjectMacro(this->Parent, "Cannot find texture: " << texturePath);
          return nullptr;
        }
      }
    }

    vTexture->MipmapOn();
    vTexture->InterpolateOn();
    vTexture->SetUseSRGBColorSpace(sRGB);

    return vTexture;
  }

  //----------------------------------------------------------------------------
  /**
   * Generate a VTK texture from an embedded ASSIMP texture
   */
  vtkSmartPointer<vtkTexture> CreateEmbeddedTexture(const aiTexture* aTexture)
  {
    vtkNew<vtkTexture> vTexture;

    if (aTexture->mHeight == 0)
    {
// CreateImageReader2FromExtension needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8211
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210729)
      std::string fileType = aTexture->achFormatHint;

      vtkSmartPointer<vtkImageReader2> reader;
      reader.TakeReference(
        vtkImageReader2Factory::CreateImageReader2FromExtension(fileType.c_str()));

      if (reader)
      {
        reader->SetMemoryBuffer(aTexture->pcData);
        reader->SetMemoryBufferLength(aTexture->mWidth);
        vTexture->SetInputConnection(reader->GetOutputPort());
      }
#else
      vtkWarningWithObjectMacro(
        this->Parent, "Embedded texture are supported by VTK >= 9.0.20210729");
#endif
    }
    else
    {
      vtkNew<vtkImageData> img;
      img->SetDimensions(aTexture->mWidth, aTexture->mHeight, 1);
      img->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

      unsigned char* imageBuffer = reinterpret_cast<unsigned char*>(img->GetScalarPointer());
      std::copy(imageBuffer, imageBuffer + 4 * aTexture->mWidth * aTexture->mHeight,
        reinterpret_cast<unsigned char*>(aTexture->pcData));

      vTexture->SetInputData(img);
    }

    return vTexture;
  }

  //----------------------------------------------------------------------------
  /**
   * Generate a VTK property from ASSIMP material
   */
  vtkSmartPointer<vtkProperty> CreateMaterial(const aiMaterial* material)
  {
    vtkNew<vtkProperty> property;

    int shadingModel;
    if (material->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == aiReturn_SUCCESS)
    {
      switch (shadingModel)
      {
        case aiShadingMode_Flat:
          property->SetInterpolationToFlat();
          break;
        case aiShadingMode_Gouraud:
        case aiShadingMode_Phong:
        case aiShadingMode_Blinn:
        case aiShadingMode_Minnaert:
          property->SetInterpolationToPhong();
          break;
        case aiShadingMode_OrenNayar:
        case aiShadingMode_CookTorrance:
        case aiShadingMode_Fresnel:
          property->SetInterpolationToPBR();
          break;
        case aiShadingMode_Toon:
        case aiShadingMode_NoShading:
          property->LightingOff();
      }
    }

    ai_real opacity;
    if (material->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
    {
      property->SetOpacity(opacity);
    }

    aiColor4D diffuse;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == aiReturn_SUCCESS)
    {
      property->SetColor(diffuse.r, diffuse.g, diffuse.b);
    }

    aiColor4D specular;
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == aiReturn_SUCCESS)
    {
      property->SetSpecularColor(specular.r, specular.g, specular.b);
    }

    aiColor4D ambient;
    if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == aiReturn_SUCCESS)
    {
      property->SetAmbientColor(ambient.r, ambient.g, ambient.b);
    }

    aiString texDiffuse;
    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texDiffuse) == aiReturn_SUCCESS)
    {
      vtkSmartPointer<vtkTexture> tex = this->CreateTexture(texDiffuse.data);
      if (tex)
      {
        property->SetTexture("diffuseTex", tex);
      }
    }

    aiString texNormal;
    if (material->GetTexture(aiTextureType_NORMALS, 0, &texNormal) == aiReturn_SUCCESS)
    {
      vtkSmartPointer<vtkTexture> tex = this->CreateTexture(texNormal.data);
      if (tex)
      {
        property->SetTexture("normalTex", tex);
      }
    }

    aiString texAlbedo;
    if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texAlbedo) == aiReturn_SUCCESS)
    {
      vtkSmartPointer<vtkTexture> tex = this->CreateTexture(texAlbedo.data, true);
      if (tex)
      {
        property->SetTexture("albedoTex", tex);
      }
    }

    aiString texEmissive;
    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texEmissive) == aiReturn_SUCCESS)
    {
      vtkSmartPointer<vtkTexture> tex = this->CreateTexture(texEmissive.data, true);
      if (tex)
      {
        property->SetTexture("emissiveTex", tex);
      }
    }

    return property;
  }

  //----------------------------------------------------------------------------
  /**
   * Generate a VTK polyData from ASSIMP mesh
   */
  vtkSmartPointer<vtkPolyData> CreateMesh(const aiMesh* mesh)
  {
    vtkNew<vtkPolyData> polyData;

    vtkNew<vtkPoints> points;
    points->SetNumberOfPoints(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      const aiVector3D& p = mesh->mVertices[i];
      points->SetPoint(i, p.x, p.y, p.z);
    }
    polyData->SetPoints(points);

    if (mesh->HasNormals())
    {
      vtkNew<vtkFloatArray> normals;
      normals->SetNumberOfComponents(3);
      normals->SetName("Normal");
      normals->SetNumberOfTuples(mesh->mNumVertices);
      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        const aiVector3D& n = mesh->mNormals[i];
        float tuple[3] = { n.x, n.y, n.z };
        normals->SetTypedTuple(i, tuple);
      }
      polyData->GetPointData()->SetNormals(normals);
    }

    // currently, VTK only supports 1 texture coordinates
    const unsigned int textureIndex = 0;
    if (mesh->HasTextureCoords(textureIndex) && mesh->mNumUVComponents[textureIndex] == 2)
    {
      vtkNew<vtkFloatArray> tcoords;
      tcoords->SetNumberOfComponents(2);
      tcoords->SetName("UV");
      tcoords->SetNumberOfTuples(mesh->mNumVertices);
      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        const aiVector3D& t = mesh->mTextureCoords[textureIndex][i];
        float tuple[2] = { t.x, t.y };
        tcoords->SetTypedTuple(i, tuple);
      }
      polyData->GetPointData()->SetTCoords(tcoords);
    }

    if (mesh->HasTangentsAndBitangents())
    {
      vtkNew<vtkFloatArray> tangents;
      tangents->SetNumberOfComponents(3);
      tangents->SetName("Tangents");
      tangents->SetNumberOfTuples(mesh->mNumVertices);
      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        const aiVector3D& t = mesh->mTangents[i];
        float tuple[3] = { t.x, t.y, t.z };
        tangents->SetTypedTuple(i, tuple);
      }
      polyData->GetPointData()->SetTangents(tangents);
    }

    if (mesh->HasVertexColors(0))
    {
      vtkNew<vtkFloatArray> colors;
      colors->SetNumberOfComponents(4);
      colors->SetName("Colors");
      colors->SetNumberOfTuples(mesh->mNumVertices);
      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        const aiColor4D& c = mesh->mColors[0][i];
        float tuple[4] = { c.r, c.g, c.b, c.a };
        colors->SetTypedTuple(i, tuple);
      }
      polyData->GetPointData()->SetScalars(colors);
    }

    vtkNew<vtkCellArray> verticesCells;
    vtkNew<vtkCellArray> linesCells;
    vtkNew<vtkCellArray> polysCells;

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      const aiFace& face = mesh->mFaces[i];

      if (face.mNumIndices == 1)
      {
        vtkIdType vId = static_cast<vtkIdType>(face.mIndices[0]);
        verticesCells->InsertNextCell(1, &vId);
      }
      else if (face.mNumIndices == 2)
      {
        vtkIdType fId[2] = { static_cast<vtkIdType>(face.mIndices[0]),
          static_cast<vtkIdType>(face.mIndices[1]) };
        linesCells->InsertNextCell(2, fId);
      }
      else
      {
        vtkIdType fId[AI_MAX_FACE_INDICES];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
          fId[j] = static_cast<vtkIdType>(face.mIndices[j]);
        }
        polysCells->InsertNextCell(face.mNumIndices, fId);
      }
    }

    polyData->SetVerts(verticesCells);
    polyData->SetLines(linesCells);
    polyData->SetPolys(polysCells);

    if (mesh->mNumBones > 0)
    {
      struct SkinData
      {
        unsigned short boneId[4] = { 0, 0, 0, 0 };
        float weight[4] = { 0, 0, 0, 0 };
        unsigned int nb = 0;
      };

      std::vector<SkinData> skinPoints(mesh->mNumVertices);

      vtkNew<vtkStringArray> bonesList;
      bonesList->SetName("Bones");

      vtkNew<vtkDoubleArray> bonesTransform;
      bonesTransform->SetName("InverseBindMatrices");
      bonesTransform->SetNumberOfComponents(16);

      for (unsigned int i = 0; i < mesh->mNumBones; i++)
      {
        aiBone* bone = mesh->mBones[i];

        bonesList->InsertValue(i, bone->mName.data);

        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
          const aiVertexWeight& vw = bone->mWeights[j];

          SkinData& data = skinPoints[vw.mVertexId];

          // F3D mapper supports only 4 weights per vertex
          if (data.nb >= 4)
          {
            continue;
          }
          data.boneId[data.nb] = i;
          data.weight[data.nb] = vw.mWeight;
          data.nb++;
        }

        vtkNew<vtkMatrix4x4> ibm;
        ConvertMatrix(bone->mOffsetMatrix, ibm);

        bonesTransform->InsertNextTypedTuple(ibm->GetData());
      }

      vtkNew<vtkFloatArray> weights;
      weights->SetName("WEIGHTS_0");
      weights->SetNumberOfComponents(4);
      weights->SetNumberOfTuples(mesh->mNumVertices);

      vtkNew<vtkUnsignedShortArray> boneIds;
      boneIds->SetName("JOINTS_0");
      boneIds->SetNumberOfComponents(4);
      boneIds->SetNumberOfTuples(mesh->mNumVertices);

      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          weights->SetTypedComponent(i, j, skinPoints[i].weight[j]);
          boneIds->SetTypedComponent(i, j, skinPoints[i].boneId[j]);
        }
      }

      polyData->GetPointData()->AddArray(weights);
      polyData->GetPointData()->AddArray(boneIds);

      polyData->GetFieldData()->AddArray(bonesList);
      polyData->GetFieldData()->AddArray(bonesTransform);
    }

    return polyData;
  }

  //----------------------------------------------------------------------------
  /**
   * Read the scene file
   */
  void ReadScene(const std::string& filePath)
  {
    try
    {
      this->Scene = this->Importer.ReadFile(filePath, aiProcess_LimitBoneWeights);
    }
    catch (const DeadlyImportError& e)
    {
      vtkWarningWithObjectMacro(this->Parent, "Assimp exception: " << e.what());
    }

    if (this->Scene)
    {
      // convert meshes to polyData
      this->Meshes.resize(this->Scene->mNumMeshes);
      for (unsigned int i = 0; i < this->Scene->mNumMeshes; i++)
      {
        this->Meshes[i] = this->CreateMesh(this->Scene->mMeshes[i]);
      }

      // read embedded textures
      this->EmbeddedTextures.resize(this->Scene->mNumTextures);
      for (unsigned int i = 0; i < this->Scene->mNumTextures; i++)
      {
        this->EmbeddedTextures[i] = this->CreateEmbeddedTexture(this->Scene->mTextures[i]);
      }

      // convert materials to properties
      this->Properties.resize(this->Scene->mNumMeshes);
      for (unsigned int i = 0; i < this->Scene->mNumMaterials; i++)
      {
        this->Properties[i] = this->CreateMaterial(this->Scene->mMaterials[i]);
      }
    }
    else
    {
      vtkWarningWithObjectMacro(this->Parent, "Assimp failed to load: " << filePath);

      auto errorDescription = this->Importer.GetErrorString();
      vtkWarningWithObjectMacro(this->Parent, "Assimp error: " << errorDescription);
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Convert a ASSIMP 4x4 matrix to a VTK 4x4 matrix
   */
  void ConvertMatrix(const aiMatrix4x4& aMat, vtkMatrix4x4* vMat)
  {
    vMat->SetElement(0, 0, aMat.a1);
    vMat->SetElement(0, 1, aMat.a2);
    vMat->SetElement(0, 2, aMat.a3);
    vMat->SetElement(0, 3, aMat.a4);
    vMat->SetElement(1, 0, aMat.b1);
    vMat->SetElement(1, 1, aMat.b2);
    vMat->SetElement(1, 2, aMat.b3);
    vMat->SetElement(1, 3, aMat.b4);
    vMat->SetElement(2, 0, aMat.c1);
    vMat->SetElement(2, 1, aMat.c2);
    vMat->SetElement(2, 2, aMat.c3);
    vMat->SetElement(2, 3, aMat.c4);
    vMat->SetElement(3, 0, aMat.d1);
    vMat->SetElement(3, 1, aMat.d2);
    vMat->SetElement(3, 2, aMat.d3);
    vMat->SetElement(3, 3, aMat.d4);
  }

  //----------------------------------------------------------------------------
  /**
   * Build recursively the node tree
   */
  void ImportNode(vtkRenderer* renderer, const aiNode* node, vtkMatrix4x4* parentMat, int level = 0)
  {
    vtkNew<vtkMatrix4x4> mat;
    vtkNew<vtkMatrix4x4> localMat;

    this->ConvertMatrix(node->mTransformation, localMat);

    vtkMatrix4x4::Multiply4x4(parentMat, localMat, mat);

    vtkNew<vtkActorCollection> actors;

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      vtkNew<vtkActor> actor;
      vtkNew<vtkPolyDataMapper> mapper;
      mapper->SetInputData(this->Meshes[node->mMeshes[i]]);
      mapper->SetColorModeToDirectScalars();

      actor->SetMapper(mapper);
      actor->SetUserMatrix(mat);
      actor->SetProperty(this->Properties[this->Scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]);

      renderer->AddActor(actor);
      actors->AddItem(actor);
    }

    for (int i = 0; i < level; i++)
    {
      this->Description += " ";
    }
    this->Description += node->mName.C_Str();
    this->Description += "\n";

    this->NodeActors.insert({ node->mName.data, actors });
    this->NodeLocalMatrix.insert({ node->mName.data, localMat });
    this->NodeGlobalMatrix.insert({ node->mName.data, mat });

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      this->ImportNode(renderer, node->mChildren[i], mat, level + 1);
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Build the node tree from the root
   */
  void ImportRoot(vtkRenderer* renderer)
  {
    if (this->Scene)
    {
      vtkNew<vtkMatrix4x4> identity;
      this->Description += "Scene Graph:\n------------\n";
      this->ImportNode(renderer, this->Scene->mRootNode, identity);

      // even if there is no animation, the bones needs to be updated
      this->UpdateBones();
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Update the actors matrix from the computed transformations
   */
  void UpdateNodeTransform(const aiNode* node, const vtkMatrix4x4* parentMat)
  {
    vtkSmartPointer<vtkMatrix4x4> localMat = this->NodeLocalMatrix[node->mName.data];

    vtkNew<vtkMatrix4x4> mat;
    vtkMatrix4x4::Multiply4x4(parentMat, localMat, mat);

    this->NodeGlobalMatrix[node->mName.data] = mat;

    // update current node actors
    vtkActorCollection* actors = this->NodeActors[node->mName.data];
    actors->InitTraversal();

    vtkActor* actor = nullptr;
    while ((actor = actors->GetNextActor()) != nullptr)
    {
      actor->SetUserMatrix(mat);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      this->UpdateNodeTransform(node->mChildren[i], mat);
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Update cameras position
   */
  void UpdateCameras()
  {
    for (auto& cam : this->Cameras)
    {
      vtkMatrix4x4* mat = this->NodeGlobalMatrix[cam.first];

      double position[4] = { 0.0, 0.0, 0.0, 1.0 };
      double up[4] = { 0.0, 1.0, 0.0, 0.0 };
      double focal[4] = { 0.0, 0.0, -1.0, 1.0 };

      mat->MultiplyPoint(position, position);
      mat->MultiplyPoint(up, up);
      mat->MultiplyPoint(focal, focal);

      cam.second->SetPosition(position);
      cam.second->SetViewUp(up);
      cam.second->SetFocalPoint(focal);
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Update lights position
   */
  void UpdateLights()
  {
    for (auto& light : this->Lights)
    {
      vtkMatrix4x4* mat = this->NodeGlobalMatrix[light.first];

      double position[4] = { 0.0, 0.0, 0.0, 1.0 };
      double focal[4] = { 0.0, 0.0, -1.0, 1.0 };

      mat->MultiplyPoint(position, position);
      mat->MultiplyPoint(focal, focal);

      light.second->SetPosition(position);
      light.second->SetFocalPoint(focal);
    }
  }

  //----------------------------------------------------------------------------
  /**
   * Update bones information for skinning
   */
  void UpdateBones()
  {
    for (auto& pairsActor : NodeActors)
    {
      vtkActorCollection* actors = pairsActor.second;
      actors->InitTraversal();

      vtkActor* actor = nullptr;
      while ((actor = actors->GetNextActor()) != nullptr)
      {
        vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());

        if (mapper)
        {
          vtkPolyData* polyData = mapper->GetInput();

          if (polyData)
          {
            vtkStringArray* bonesList =
              vtkStringArray::SafeDownCast(polyData->GetFieldData()->GetAbstractArray("Bones"));
            vtkDoubleArray* bonesTransform = vtkDoubleArray::SafeDownCast(
              polyData->GetFieldData()->GetArray("InverseBindMatrices"));
            if (bonesList && bonesTransform)
            {
              vtkIdType nbBones = bonesList->GetNumberOfValues();

              if (nbBones > 0)
              {
                std::vector<float> vec;
                vec.reserve(16 * nbBones);

                vtkNew<vtkMatrix4x4> inverseRoot;
                actor->GetUserMatrix()->DeepCopy(inverseRoot);
                inverseRoot->Invert();

                for (vtkIdType i = 0; i < nbBones; i++)
                {
                  std::string boneName = bonesList->GetValue(i);

                  vtkNew<vtkMatrix4x4> boneMat;
                  bonesTransform->GetTypedTuple(i, boneMat->GetData());

                  vtkMatrix4x4::Multiply4x4(this->NodeGlobalMatrix[boneName], boneMat, boneMat);
                  vtkMatrix4x4::Multiply4x4(inverseRoot, boneMat, boneMat);

                  for (int j = 0; j < 4; j++)
                  {
                    for (int k = 0; k < 4; k++)
                    {
                      vec.push_back(static_cast<float>(boneMat->GetElement(k, j)));
                    }
                  }
                }

                vtkShaderProperty* shaderProp = actor->GetShaderProperty();
                vtkUniforms* uniforms = shaderProp->GetVertexCustomUniforms();
                uniforms->RemoveAllUniforms();
                uniforms->SetUniformMatrix4x4v(
                  "jointMatrices", static_cast<int>(nbBones), vec.data());
              }
            }
          }
        }
      }
    }
  }

  Assimp::Importer Importer;
  const aiScene* Scene;
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
  vtkF3DAssimpImporter* Parent;
};

//----------------------------------------------------------------------------
vtkF3DAssimpImporter::vtkF3DAssimpImporter()
  : Internals(new vtkF3DAssimpImporter::vtkInternals(this))
{
}

//----------------------------------------------------------------------------
vtkF3DAssimpImporter::~vtkF3DAssimpImporter() = default;

//----------------------------------------------------------------------------
int vtkF3DAssimpImporter::ImportBegin()
{
  this->Internals->ReadScene(this->FileName);

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::ImportActors(vtkRenderer* renderer)
{
  this->Internals->ImportRoot(renderer);
}

//----------------------------------------------------------------------------
std::string vtkF3DAssimpImporter::GetOutputsDescription()
{
  return this->Internals->Description;
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::UpdateTimeStep(double timestep)
{
  if (this->Internals->ActiveAnimation < 0 ||
    this->Internals->ActiveAnimation >= this->GetNumberOfAnimations())
  {
    return;
  }

  // get the animation tick
  timestep *= this->Internals->Scene->mAnimations[0]->mTicksPerSecond;

  aiAnimation* anim = this->Internals->Scene->mAnimations[this->Internals->ActiveAnimation];

  Assimp::Interpolator<aiVectorKey> vectorInterpolator;
  Assimp::Interpolator<aiQuatKey> quaternionInterpolator;

  for (unsigned int nodeChannelId = 0; nodeChannelId < anim->mNumChannels; nodeChannelId++)
  {
    aiNodeAnim* nodeAnim = anim->mChannels[nodeChannelId];

    aiVector3D translation;
    aiVector3D scaling;
    aiQuaternion quaternion;

    aiVectorKey* positionKey = std::lower_bound(nodeAnim->mPositionKeys,
      nodeAnim->mPositionKeys + nodeAnim->mNumPositionKeys, timestep,
      [](const aiVectorKey& key, const double& time) { return key.mTime < time; });

    if (positionKey == nodeAnim->mPositionKeys)
    {
      translation = positionKey->mValue;
    }
    else if (positionKey == nodeAnim->mPositionKeys + nodeAnim->mNumPositionKeys)
    {
      translation = (positionKey - 1)->mValue;
    }
    else
    {
      aiVectorKey* prev = positionKey - 1;
      ai_real d = (timestep - prev->mTime) / (positionKey->mTime - prev->mTime);
      vectorInterpolator(translation, *prev, *positionKey, d);
    }

    aiQuatKey* rotationKey = std::lower_bound(nodeAnim->mRotationKeys,
      nodeAnim->mRotationKeys + nodeAnim->mNumRotationKeys, timestep,
      [](const aiQuatKey& key, const double& time) { return key.mTime < time; });

    if (rotationKey == nodeAnim->mRotationKeys)
    {
      quaternion = rotationKey->mValue;
    }
    else if (rotationKey == nodeAnim->mRotationKeys + nodeAnim->mNumRotationKeys)
    {
      quaternion = (rotationKey - 1)->mValue;
    }
    else
    {
      aiQuatKey* prev = rotationKey - 1;
      ai_real d = (timestep - prev->mTime) / (rotationKey->mTime - prev->mTime);
      quaternionInterpolator(quaternion, *prev, *rotationKey, d);
    }

    aiVectorKey* scalingKey =
      std::lower_bound(nodeAnim->mScalingKeys, nodeAnim->mScalingKeys + nodeAnim->mNumScalingKeys,
        timestep, [](const aiVectorKey& key, const double& time) { return key.mTime < time; });

    if (scalingKey == nodeAnim->mScalingKeys)
    {
      scaling = scalingKey->mValue;
    }
    else if (scalingKey == nodeAnim->mScalingKeys + nodeAnim->mNumScalingKeys)
    {
      scaling = (scalingKey - 1)->mValue;
    }
    else
    {
      aiVectorKey* prev = scalingKey - 1;
      ai_real d = (timestep - prev->mTime) / (scalingKey->mTime - prev->mTime);
      vectorInterpolator(scaling, *prev, *scalingKey, d);
    }

    vtkMatrix4x4* transform = this->Internals->NodeLocalMatrix[nodeAnim->mNodeName.data];

    if (transform)
    {
      // Initialize quaternion
      vtkQuaternion<double> rotation;
      rotation.Set(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
      rotation.Normalize();

      double rotationMatrix[3][3];
      rotation.ToMatrix3x3(rotationMatrix);

      // Apply transformations
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          transform->SetElement(i, j, scaling[j] * rotationMatrix[i][j]);
        }
        transform->SetElement(i, 3, translation[i]);
      }
    }
  }

  vtkNew<vtkMatrix4x4> identity;
  this->Internals->UpdateNodeTransform(this->Internals->Scene->mRootNode, identity);

  this->Internals->UpdateBones();
  this->Internals->UpdateCameras();
  this->Internals->UpdateLights();
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DAssimpImporter::GetNumberOfAnimations()
{
  return this->Internals->Scene ? this->Internals->Scene->mNumAnimations : 0;
}

//----------------------------------------------------------------------------
std::string vtkF3DAssimpImporter::GetAnimationName(vtkIdType animationIndex)
{
  return this->Internals->Scene->mAnimations[animationIndex]->mName.C_Str();
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::EnableAnimation(vtkIdType animationIndex)
{
  this->Internals->ActiveAnimation = animationIndex;
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::DisableAnimation(vtkIdType vtkNotUsed(animationIndex))
{
  this->Internals->ActiveAnimation = 0;
}

//----------------------------------------------------------------------------
bool vtkF3DAssimpImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  return this->Internals->ActiveAnimation == animationIndex;
}

// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200912)
//----------------------------------------------------------------------------
bool vtkF3DAssimpImporter::GetTemporalInformation(vtkIdType animationIndex, double frameRate,
  int& nbTimeSteps, double timeRange[2], vtkDoubleArray* timeSteps)
{
  double duration = this->Internals->Scene->mAnimations[animationIndex]->mDuration;
  double fps = this->Internals->Scene->mAnimations[animationIndex]->mTicksPerSecond;

  this->Internals->Description += "Animation \"";
  this->Internals->Description += this->GetAnimationName(animationIndex);
  this->Internals->Description += "\": ";
  this->Internals->Description += std::to_string(duration);
  this->Internals->Description += " ticks, ";
  this->Internals->Description += std::to_string(fps);
  this->Internals->Description += " fps.\n";

  if (fps == 0.0)
  {
    fps = frameRate;
  }

  timeRange[0] = 0.0;
  timeRange[1] = duration / fps;

  timeSteps->SetNumberOfComponents(1);
  timeSteps->SetNumberOfTuples(0);

  nbTimeSteps = 0;

  for (double time = 0.0; time < timeRange[1]; time += (1.0 / frameRate))
  {
    timeSteps->InsertNextTuple(&time);
    nbTimeSteps++;
  }

  return true;
}
#endif

// Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200912)
//----------------------------------------------------------------------------
vtkIdType vtkF3DAssimpImporter::GetNumberOfCameras()
{
  return this->Internals->Cameras.size();
}

//----------------------------------------------------------------------------
std::string vtkF3DAssimpImporter::GetCameraName(vtkIdType camIndex)
{
  return this->Internals->Cameras[camIndex].first;
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::SetCamera(vtkIdType camIndex)
{
  this->Internals->ActiveCameraIndex = camIndex;
}
#endif

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::ImportCameras(vtkRenderer* renderer)
{
  this->Internals->ImportCameras(renderer);
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::ImportLights(vtkRenderer* renderer)
{
  this->Internals->ImportLights(renderer);
}

//----------------------------------------------------------------------------
void vtkF3DAssimpImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
