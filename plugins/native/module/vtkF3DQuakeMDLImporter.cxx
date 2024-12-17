#include "vtkF3DQuakeMDLImporter.h"
#include <vtkImporter.h>
#include <vtkActorCollection.h>
#include <vtkAnimationScene.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOpenGLTexture.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkScenePicker.h>
#include <vtkTable.h>
#include <vtkTextureObject.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DQuakeMDLImporter);

//----------------------------------------------------------------------------
class vtkF3DQuakeMDLImporter::vtkInternals
{
public:
  //----------------------------------------------------------------------------
  explicit vtkInternals(vtkF3DQuakeMDLImporter* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  void ImportCameras(vtkRenderer* vtkNotUsed(renderer))
  {
  }

  //----------------------------------------------------------------------------
  void ImportLights(vtkRenderer* vtkNotUsed(renderer))
  {
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkTexture> CreateTexture(const std::vector<unsigned char> buffer, int& offset,
    int skinWidth, int skinHeight, int nbSkins, int selectedSkinIndex)
  {
    vtkNew<vtkTexture> texture;
    texture->InterpolateOn();

    // Read textures.
    struct mixed_pointer_array
    {
      int group;
      unsigned char* skin;
    };
    std::vector<mixed_pointer_array> skins = std::vector<mixed_pointer_array>(nbSkins);
    for (int i = 0; i < nbSkins; i++)
    {
      int* group = (int*) (buffer.data() + offset);
      if (*group == 0)
      {
        skins[i].group = 0;
        skins[i].skin = (unsigned char*) (buffer.data() + 4 + offset);
        offset += 4 + (skinWidth) * (skinHeight);
      }
      else
      {
        skins[i].group = 1;
        int nb = *(int*) (buffer.data() + offset + 4);
        skins[i].skin = (unsigned char*) (buffer.data() + 4 + nb * 4 + offset);
        offset += 4 + nb * 4 + nb * (skinWidth) * (skinHeight);
      }
    }

    // Copy to imageData
    vtkNew<vtkImageData> img;
    img->SetDimensions(skinWidth, skinHeight, 1);
    img->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    unsigned char* selectedSkin = skins[selectedSkinIndex].skin;
    for (int i = 0; i < skinHeight; i++)
    {
      for (int j = 0; j < skinWidth; j++)
      {
        unsigned char index = *(unsigned char*) (selectedSkin + i * skinWidth + j);
        unsigned char* ptr = (unsigned char*) (img->GetScalarPointer(j, i, 0));
        ptr[0] = F3DMDLDefaultColorMap[index][0]; // R
        ptr[1] = F3DMDLDefaultColorMap[index][1]; // G
        ptr[2] = F3DMDLDefaultColorMap[index][2]; // B
      }
    }
    texture->SetInputData(img);
    return texture;
  }

  // Header definition,
  struct mdl_header_t
  {
    int IDPO;
    int version;
    float scale[3];
    float translation[3];
    float boundingRadius;
    float eyePosition[3];
    int numSkins;
    int skinWidth;
    int skinHeight;
    int numVertices;
    int numTriangles;
    int numFrames;
    int syncType;
    int stateFlags;
    float size;
  };


  //----------------------------------------------------------------------------
  void CreateMesh(std::vector<unsigned char> buffer, int offset, mdl_header_t* header)
  {
    // Read texture coordinates
    struct mdl_texcoord_t
    {
      int onseam;
      int s;
      int t;
    };
    mdl_texcoord_t* texcoords = (mdl_texcoord_t*) (buffer.data() + offset);
    offset += 12 * header->numVertices;
    // Read triangles
    struct mdl_triangle_t
    {
      int facesfront; /* 0 = backface, 1 = frontface */
      int vertex[3];  /* vertex indices */
    };
    mdl_triangle_t* triangles = (mdl_triangle_t*) (buffer.data() + offset);
    offset += 16 * header->numTriangles;
    // Read frames
    struct mdl_vertex_t // 4 bytes
    {
      unsigned char v[3];
      unsigned char normalIndex;
    };
    struct mdl_simpleframe_t // 24 + nbVertices bytes
    {
      mdl_vertex_t bboxmin;
      mdl_vertex_t bboxmax;
      char name[16];
      mdl_vertex_t verts[1024]; // Maximum capacity is 1024 vertices
    };
    struct plugin_frame_pointer
    {
      int* type;
      int* nb;
      float* time;
      mdl_simpleframe_t* frames;
    };
    std::vector<plugin_frame_pointer> framePtr = std::vector<plugin_frame_pointer>(header->numFrames);
    for (int i = 0; i < header->numFrames; i++)
    {
      int* type = (int*) (buffer.data() + offset);
      if (*type == 0)
      {
        framePtr[i].type = type;
        framePtr[i].nb = nullptr;
        framePtr[i].time = nullptr;
        framePtr[i].frames = (mdl_simpleframe_t*) (buffer.data() + 4 + offset);
        offset += 4 + 24 + 4 * (header->numVertices);
      }
      else
      {
        framePtr[i].type = type;
        framePtr[i].nb = (int*) (buffer.data() + 4 + offset);
        // mdl_vertex_t* min = reinterpret_cast<mdl_vertex_t*>(buffer.data() + 8 + offset);
        // mdl_vertex_t* max = reinterpret_cast<mdl_vertex_t*>(buffer.data() + 12 + offset);
        // float* time = framePtr[i].time = reinterpret_cast<float*>(buffer.data() + 16 + offset);
        framePtr[i].frames =
          (mdl_simpleframe_t*) (buffer.data() + 16 + 4 * (*framePtr[i].nb) + offset);
        offset += 16 + (*framePtr[i].nb) * 4;
        for (int j = 0; j < *framePtr[i].nb; j++)
        {
          offset += 24 + 4 * header->numVertices;
        }
      }
    }
    // Draw cells
    vtkNew<vtkCellArray> cells;
    cells->Allocate(header->numTriangles);
    vtkNew<vtkFloatArray> textureCoordinates;
    textureCoordinates->SetNumberOfComponents(2);
    textureCoordinates->SetName("TextureCoordinates");
    textureCoordinates->Allocate(header->numTriangles * 3);
    struct plugin_texture_coords
    {
      float s;
      float t;
      int id;
    };
    std::vector<plugin_texture_coords> coords = std::vector<plugin_texture_coords>(header->numTriangles * 3);
    for (int i = 0; i < header->numTriangles; i++)
    {
      vtkIdType vertexNum[3];
      for (int j = 0; j < 3; j++)
      {
        vertexNum[j] = triangles[i].vertex[j];
        // int onseam_correct = 1;
        float s = texcoords[triangles[i].vertex[j]].s;
        float t = texcoords[triangles[i].vertex[j]].t;
        if (!triangles[i].facesfront && texcoords[triangles[i].vertex[j]].onseam)
        {
          s = s + header->skinWidth * 0.5f;
        }
        s = (s + 0.5) / header->skinWidth;
        t = (t + 0.5) / header->skinHeight;
        coords[3 * i + j].s = s;
        coords[3 * i + j].t = t;
        coords[3 * i + j].id = triangles[i].vertex[j];
        float st[2] = { s, t };
        textureCoordinates->InsertNextTuple(st);
      }
      vtkIdType t[3] = { i * 3, i * 3 + 1, i * 3 + 2 };
      cells->InsertNextCell(3, t);
    }

    // Draw vertices
    std::string frameName = "";
    int frameIndex = 0;
    for (int frameNum = 0; frameNum < header->numFrames; frameNum++)
    {
      vtkNew<vtkPoints> vertices;
      vertices->Allocate(header->numTriangles * 3);
      vtkNew<vtkFloatArray> normals;
      normals->SetNumberOfComponents(3);
      normals->Allocate(header->numTriangles * 3 * 3);

      plugin_frame_pointer selectedFrame = framePtr[frameNum];
      if (*selectedFrame.type == 0)
      {
        for (int i = 0; i < header->numTriangles; i++)
        {
          vtkIdType vertexNum[3];
          for (int j = 0; j < 3; j++)
          {
            vertexNum[j] = triangles[i].vertex[j];
            double v[3] = { double(selectedFrame.frames->verts[vertexNum[j]].v[0]),
              double(selectedFrame.frames->verts[vertexNum[j]].v[1]),
              double(selectedFrame.frames->verts[vertexNum[j]].v[2]) };
            for (int k = 0; k < 3; k++)
            {
              v[k] = v[k] * header->scale[k] + header->translation[k];
            }
            vertices->InsertPoint(i * 3 + j, v);
            int normalIndex = selectedFrame.frames->verts[vertexNum[j]].normalIndex;
            normals->SetTuple3(i * 3 + j, F3DMDLNormalVectors[normalIndex][0] / 255.0,
              F3DMDLNormalVectors[normalIndex][1] / 255.0, F3DMDLNormalVectors[normalIndex][2] / 255.0);
          }
        }
        vtkNew<vtkPolyData> mesh;
        mesh->SetPoints(vertices);
        mesh->SetPolys(cells);
        mesh->GetPointData()->SetTCoords(textureCoordinates);
        //        mesh->GetPointData()->SetNormals(normals);
        Mesh.emplace_back(mesh);
        std::string meshName = std::string(selectedFrame.frames->name);
        for (std::size_t i = 0; i < meshName.size(); i++)
        {
          if (meshName[i] >= '0' && meshName[i] <= '9')
          {
            meshName = meshName.substr(0, i);
            break;
          }
        }
        if (frameNum == 0)
        {
          frameName = meshName;
          NumberOfAnimations++;
          AnimationNames.emplace_back(meshName);
        }
        else if (meshName != frameName)
        {
          frameIndex++;
          frameName = meshName;
          NumberOfAnimations++;
          AnimationNames.emplace_back(meshName);
        }
        GroupAndTimeVal.emplace_back(std::make_pair( frameIndex, 0.0 ));
      }
      else
      {
        for (int groupFrameNum = 0; groupFrameNum < *selectedFrame.nb; groupFrameNum++)
        {
          for (int i = 0; i < header->numTriangles; i++)
          {
            vtkIdType vertexNum[3];
            for (int j = 0; j < 3; j++)
            {
              vertexNum[j] = triangles[i].vertex[j];
              double v[3] = { double(selectedFrame.frames[groupFrameNum].verts[vertexNum[j]].v[0]),
                double(selectedFrame.frames[groupFrameNum].verts[vertexNum[j]].v[1]),
                double(selectedFrame.frames[groupFrameNum].verts[vertexNum[j]].v[2]) };
              for (int k = 0; k < 3; k++)
              {
                v[k] = v[k] * header->scale[k] + header->translation[k];
              }
              vertices->InsertPoint(i * 3 + j, v);
              int normalIndex = selectedFrame.frames[groupFrameNum].verts[vertexNum[j]].normalIndex;
              normals->SetTuple3(i * 3 + j, F3DMDLNormalVectors[normalIndex][0] / 255.0,
                F3DMDLNormalVectors[normalIndex][1] / 255.0, F3DMDLNormalVectors[normalIndex][2] / 255.0);
            }
          }
          vtkNew<vtkPolyData> mesh;
          mesh->SetPoints(vertices);
          mesh->SetPolys(cells);
          mesh->GetPointData()->SetTCoords(textureCoordinates);
          mesh->GetPointData()->SetNormals(normals);
          Mesh.emplace_back(mesh);
          std::string meshName = std::string(selectedFrame.frames[groupFrameNum].name);
          for (std::size_t i = 0; i < meshName.size(); i++)
          {
            if (meshName[i] >= '0' && meshName[i] <= '9')
            {
              meshName = meshName.substr(0, i - 1);
              break;
            }
          }
          if (frameNum == 0)
          {
            frameName = meshName;
            NumberOfAnimations++;
            AnimationNames.emplace_back(meshName);
          }
          else if (meshName != frameName)
          {
            frameIndex++;
            NumberOfAnimations++;
            AnimationNames.emplace_back(meshName);
            frameName = meshName;
          }
          GroupAndTimeVal.emplace_back(std::make_pair(frameIndex, 0.0));
        }
      }
    }

    // Add interpolated frames
    for (std::size_t i = 0; i < Mesh.size() - 1; i++)
    {
      if (GroupAndTimeVal[i + 1].first != GroupAndTimeVal[i].first)
      {
        continue;
      }
      else
      {
        vtkNew<vtkPoints> vertices;
        vertices->Allocate(header->numTriangles * 3);
        for (int j = 0; j < header->numTriangles * 3; j++)
        {
          double* v_0 = Mesh[i]->GetPoint(j);
          double* v_1 = Mesh[i + 1]->GetPoint(j);
          double interp[3];
          interp[0] = v_0[0] + 0.5 * (v_1[0] - v_0[0]);
          interp[1] = v_0[1] + 0.5 * (v_1[1] - v_0[1]);
          interp[2] = v_0[2] + 0.5 * (v_1[2] - v_0[2]);
          vertices->InsertPoint(j, interp);
        }
        vtkNew<vtkPolyData> mesh;
        mesh->SetPoints(vertices);
        mesh->SetPolys(cells);
        mesh->GetPointData()->SetTCoords(textureCoordinates);
        Mesh.insert(Mesh.begin() + i, mesh);
        std::pair<int, float> pair = std::make_pair(GroupAndTimeVal[i].first,
          (GroupAndTimeVal[i].second + GroupAndTimeVal[i + 1].second) / 2);
        GroupAndTimeVal.insert(GroupAndTimeVal.begin() + i, pair);
        i++;
      }
    }

  }

  //----------------------------------------------------------------------------
  bool ReadScene(const std::string& filePath)
  {
    std::ifstream inputStream(filePath, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});
    // Read header
    mdl_header_t* header = reinterpret_cast<mdl_header_t*>(buffer.data());
    int offset = 84;

    // Set textures
    Texture = this->CreateTexture(
      buffer, offset, header->skinWidth, header->skinHeight, header->numSkins, 0);

    // Set polyData
    this->CreateMesh(buffer, offset, header);

    return true;
  }

  //----------------------------------------------------------------------------
  void UpdateTimeStep(double timeValue)
  {
    int frameIndex = (int) floor(FrameRate * abs(timeValue)) % (int)ActiveFrames.size();
    int currentFrame = ActiveFrames[frameIndex];
    Mapper->SetInputData(Mesh[currentFrame]);
    LastRenderTime = timeValue;
  }

  //----------------------------------------------------------------------------
  void EnableAnimation(vtkIdType animationIndex)
  {
    int firstFrameIndex = std::distance(GroupAndTimeVal.begin(),
      std::find_if(GroupAndTimeVal.begin(), GroupAndTimeVal.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first == animationIndex; }));
    int lastFrameIndex = std::distance(GroupAndTimeVal.begin(),
      std::find_if(GroupAndTimeVal.begin(), GroupAndTimeVal.end(),
      [animationIndex](const std::pair<int, float> pair)
      { return pair.first > animationIndex; }));
    firstFrameIndex = firstFrameIndex <= GroupAndTimeVal.size() ? firstFrameIndex : 0;
    lastFrameIndex =
      lastFrameIndex <= GroupAndTimeVal.size() ? lastFrameIndex - 1 : lastFrameIndex;
    for (int i = firstFrameIndex; i <= lastFrameIndex; i++)
    {
      ActiveFrames.emplace_back(i);
    }
    ActiveAnimationId.emplace_back(animationIndex);
  }

  //----------------------------------------------------------------------------
  void DisableAnimation(vtkIdType animationIndex)
  {
    int firstFrameIndex = std::distance(GroupAndTimeVal.begin(),
      std::find_if(GroupAndTimeVal.begin(), GroupAndTimeVal.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first == animationIndex; }));
    int lastFrameIndex = std::distance(GroupAndTimeVal.begin(),
      std::find_if(GroupAndTimeVal.begin(), GroupAndTimeVal.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first > animationIndex; }));
    firstFrameIndex = firstFrameIndex <= GroupAndTimeVal.size() ? firstFrameIndex : 0;
    lastFrameIndex = lastFrameIndex <= GroupAndTimeVal.size() ? lastFrameIndex - 1 : lastFrameIndex;
    for (int i = firstFrameIndex; i <= lastFrameIndex; i++)
    {
      ActiveFrames.erase(std::remove(ActiveFrames.begin(), ActiveFrames.end(), i), ActiveFrames.end());
    }
    ActiveAnimationId.erase(
      std::remove(ActiveAnimationId.begin(), ActiveAnimationId.end(), animationIndex), ActiveAnimationId.end());
  }

  //----------------------------------------------------------------------------
  void ImportActors(vtkRenderer* renderer)
  {
    vtkNew<vtkActor> actor;
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(Mesh[0]);
    actor->SetMapper(mapper);
    actor->SetTexture(Texture);
    renderer->AddActor(actor);
    renderer->SetBackground(0, 0, 0);
    Actor = actor;
    Mapper = mapper;
  }

  //----------------------------------------------------------------------------
  void SetFrameRate(double frameRate)
  {
    FrameRate = frameRate;
  }

  //----------------------------------------------------------------------------
  void GetTimeRange(vtkIdType vtkNotUsed(animationIndex), double timeRange[2])
  {
    timeRange[0] = 0.0;
    timeRange[1] = (1.0 / FrameRate) * GroupAndTimeVal.size();
  }

  vtkF3DQuakeMDLImporter* Parent;
  std::string Description;
  vtkSmartPointer<vtkActor> Actor;
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  std::vector<vtkSmartPointer<vtkPolyData>> Mesh;
  std::vector<std::pair<int, float>> GroupAndTimeVal;
  std::vector<std::string> AnimationNames;
  std::vector<int> ActiveFrames;
  std::vector<int> ActiveAnimationId;
  int NumberOfAnimations = 0;
  double LastRenderTime = 0.0;
  double FrameRate = 60.0;
  vtkSmartPointer<vtkTexture> Texture;
  vtkSmartPointer<vtkFloatArray> TextureCoords;
};

//----------------------------------------------------------------------------
vtkF3DQuakeMDLImporter::vtkF3DQuakeMDLImporter()
  : Internals(new vtkF3DQuakeMDLImporter::vtkInternals(this)){

  };

//----------------------------------------------------------------------------
int vtkF3DQuakeMDLImporter::ImportBegin()
{
  return this->Internals->ReadScene(this->FileName);
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::ImportActors(vtkRenderer* renderer)
{
  this->Internals->ImportActors(renderer);
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetOutputsDescription()
{
  return this->Internals->Description;
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::UpdateTimeStep(double timeValue)
{
  return this->Internals->UpdateTimeStep(timeValue);
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfAnimations()
{
  return this->Internals->NumberOfAnimations;
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetAnimationName(vtkIdType animationIndex)
{
  if (animationIndex < (int) this->Internals->AnimationNames.size())
  {
    return this->Internals->AnimationNames[animationIndex];
  }
  else
  {
    return "";
  }
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::EnableAnimation(vtkIdType animationIndex)
{
  this->Internals->EnableAnimation(animationIndex);
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::DisableAnimation(vtkIdType animationIndex)
{
  this->Internals->DisableAnimation(animationIndex);
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  return std::count(this->Internals->ActiveAnimationId.begin(), this->Internals->ActiveAnimationId.end(), animationIndex) > 0;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(vtkIdType animationIndex,
  double frameRate, int& nbTimeSteps, double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  Internals->SetFrameRate(frameRate);
  Internals->GetTimeRange(animationIndex, timeRange);
  nbTimeSteps = (int) Internals->ActiveFrames.size();
  return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfCameras()
{
  return 1;
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetCameraName(vtkIdType vtkNotUsed(camIndex))
{
  return "Camera";
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::SetCamera(vtkIdType vtkNotUsed(camIndex))
{
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::ImportCameras(vtkRenderer* renderer)
{
  this->Internals->ImportCameras(renderer);
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::ImportLights(vtkRenderer* renderer)
{
  this->Internals->ImportLights(renderer);
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::SetFileName(std::string fileName)
{
  this->FileName = fileName;
}
