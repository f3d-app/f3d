#include "vtkF3DQuakeMDLImporter.h"
#include "vtkF3DQuakeMDLImporterConstants.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkOpenGLTexture.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>

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
  vtkSmartPointer<vtkTexture> CreateTexture(const std::vector<unsigned char>& buffer, int& offset,
    int skinWidth, int skinHeight, int nbSkins, int selectedSkinIndex)
  {
    vtkNew<vtkTexture> texture;
    texture->InterpolateOn();

    // Read textures.
    struct mixed_pointer_array
    {
      int group;
      const unsigned char* skin;
    };
    std::vector<mixed_pointer_array> skins = std::vector<mixed_pointer_array>(nbSkins);
    for (int i = 0; i < nbSkins; i++)
    {
      skins[i].group = *reinterpret_cast<const int*>(buffer.data() + offset);
      if (skins[i].group == 0)
      {
        skins[i].skin = buffer.data() + 4 + offset;
        offset += 4 + skinWidth * skinHeight;
      }
      else
      {
        int nb = *reinterpret_cast<const int*>(buffer.data() + offset + 4);
        skins[i].skin = buffer.data() + 4 + nb * 4 + offset;
        offset += 4 + nb * 4 + nb * skinWidth * skinHeight;
      }
    }

    // Copy to imageData
    vtkNew<vtkImageData> img;
    img->SetDimensions(skinWidth, skinHeight, 1);
    img->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    const unsigned char* selectedSkin = skins[selectedSkinIndex].skin;
    for (int i = 0; i < skinHeight; i++)
    {
      for (int j = 0; j < skinWidth; j++)
      {
        unsigned char index = *(selectedSkin + i * skinWidth + j);
        unsigned char* ptr = static_cast<unsigned char*>(img->GetScalarPointer(j, i, 0));
        ptr[0] = F3DMDLDefaultColorMap[index][0]; // R
        ptr[1] = F3DMDLDefaultColorMap[index][1]; // G
        ptr[2] = F3DMDLDefaultColorMap[index][2]; // B
      }
    }
    texture->SetInputData(img);
    return texture;
  }

  // Header definition
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
  void CreateMesh(const std::vector<unsigned char>& buffer, int offset, mdl_header_t* header)
  {
    // Read texture coordinates
    struct mdl_texcoord_t
    {
      int onseam;
      int s;
      int t;
    };
    const mdl_texcoord_t* texcoords = reinterpret_cast<const mdl_texcoord_t*>(buffer.data() + offset);
    offset += 12 * header->numVertices;

    // Read triangles
    struct mdl_triangle_t 
    {
      int facesfront; // 0 = backface, 1 = frontface
      int vertex[3];  // vertex indices
    };
    const mdl_triangle_t* triangles = reinterpret_cast<const mdl_triangle_t*>(buffer.data() + offset);
    offset += 16 * header->numTriangles;

    // Simple vertex and normal
    struct mdl_vertex_t
    {
      unsigned char v[3];
      unsigned char normalIndex;
    };
    // Simple frame
    struct mdl_simpleframe_t // 24 + nbVertices bytes
    {
      mdl_vertex_t bboxmin; // bounding box min
      mdl_vertex_t bboxmax; // bounding box max
      char name[16];  // frame name
      mdl_vertex_t verts[1024]; // vertex list of the frame, maximum capacity is 1024
    };
    // Struct containing frame type and data
    struct plugin_frame_pointer
    {
      const int* type; // 0 = simple frame, !0 = group frames
      const int* nb; // "number", size of *time and *frames arrays
      const float* time;  // time duration for each frame
      const mdl_simpleframe_t* frames;  // simple frame list
    };
    // Read frames
    std::vector<plugin_frame_pointer> framePtr =
      std::vector<plugin_frame_pointer>(header->numFrames);
    std::vector<std::vector<int>> frameOffsets = std::vector<std::vector<int>>();
    for (int i = 0; i < header->numFrames; i++)
    {
      framePtr[i].type = reinterpret_cast<const int*>(buffer.data() + offset);
      if (*framePtr[i].type == 0)
      {
        framePtr[i].nb = nullptr;
        framePtr[i].time = nullptr;
        framePtr[i].frames = reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + 4 + offset);
        offset += 4 + 24 + 4 * header->numVertices;
      }
      else
      {
        framePtr[i].nb = reinterpret_cast<const int*>(buffer.data() + 4 + offset);
        framePtr[i].time = reinterpret_cast<const float*>(buffer.data() + 16 + offset);
        framePtr[i].frames = reinterpret_cast<const mdl_simpleframe_t*>(
          buffer.data() + 16 + 4 * (*framePtr[i].nb) + offset);
        frameOffsets.emplace_back(std::vector<int>());
        for (int j = 0; j < *framePtr[i].nb; j++)
        {
          frameOffsets[i].emplace_back(
            16 + 4 * (*framePtr[i].nb) + j * 4 * (header->numVertices + 6) + offset);
        }
        offset += 16 + (*framePtr[i].nb) * 4 + (24 + 4 * header->numVertices) * (*framePtr[i].nb);
      }
    }

    // Draw cells and scale texture coordinates
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
    };
    std::vector<plugin_texture_coords> coords =
      std::vector<plugin_texture_coords>(header->numTriangles * 3);
    for (int i = 0; i < header->numTriangles; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        float s = texcoords[triangles[i].vertex[j]].s;
        float t = texcoords[triangles[i].vertex[j]].t;
        if (!triangles[i].facesfront && texcoords[triangles[i].vertex[j]].onseam)
        {
          s = s + header->skinWidth * 0.5f; // Backface
        }
        // Scale s and t to range from 0.0 to 1.0
        s = (s + 0.5) / header->skinWidth;
        t = (t + 0.5) / header->skinHeight;
        coords[3 * i + j].s = s;
        coords[3 * i + j].t = t;
        float st[2] = { s, t };
        textureCoordinates->InsertNextTuple(st);
      }
      vtkIdType t[3] = { i * 3, i * 3 + 1, i * 3 + 2 };
      cells->InsertNextCell(3, t);
    }

    // Draw frames
    std::string frameName = "";
    int animationId = 0;
    for (int frameNum = 0; frameNum < header->numFrames; frameNum++)
    {
      plugin_frame_pointer selectedFrame = framePtr[frameNum];
      int numGroupFrames = *selectedFrame.type == 0 ? 1 : *selectedFrame.nb;
      for (int groupFrameNum = 0; groupFrameNum < numGroupFrames; groupFrameNum++)
      {
        vtkNew<vtkPoints> vertices;
        vertices->Allocate(header->numTriangles * 3);
        vtkNew<vtkFloatArray> normals;
        normals->SetNumberOfComponents(3);
        normals->Allocate(header->numTriangles * 3 * 3);
        const mdl_simpleframe_t* selectedSimpleFrame = numGroupFrames > 1
          ? reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + frameOffsets[frameNum][groupFrameNum])
          : selectedFrame.frames;
        for (int i = 0; i < header->numTriangles; i++)
        {
          vtkIdType vertexNum[3];
          for (int j = 0; j < 3; j++)
          {
            vertexNum[j] = triangles[i].vertex[j];
            double v[3] = { double(selectedSimpleFrame->verts[vertexNum[j]].v[0]),
              double(selectedSimpleFrame->verts[vertexNum[j]].v[1]),
              double(selectedSimpleFrame->verts[vertexNum[j]].v[2]) };
            // Calculate real vertex position
            for (int k = 0; k < 3; k++)
            {
              v[k] = v[k] * header->scale[k] + header->translation[k];
            }
            vertices->InsertPoint(i * 3 + j, v);
            // Normal vector
            int normalIndex = selectedSimpleFrame->verts[vertexNum[j]].normalIndex;
            normals->SetTuple3(i * 3 + j, F3DMDLNormalVectors[normalIndex][0] / 255.0,
              F3DMDLNormalVectors[normalIndex][1] / 255.0,
              F3DMDLNormalVectors[normalIndex][2] / 255.0);
          }
        }
        vtkNew<vtkPolyData> mesh;
        mesh->SetPoints(vertices);
        mesh->SetPolys(cells);
        mesh->GetPointData()->SetTCoords(textureCoordinates);
        mesh->GetPointData()->SetNormals(normals);
        this->Mesh.emplace_back(mesh);
        // Group meshes by animation name
        // Another array contains animation ids and time values
        std::string meshName = std::string(selectedSimpleFrame->name);
        for (std::size_t i = 0; i < meshName.size(); i++)
        {
          if (meshName[i] >= '0' && meshName[i] <= '9')
          {
            meshName = meshName.substr(0, i);
            break;
          }
        }
        if (frameNum == 0 && groupFrameNum == 0)
        {
          frameName = meshName;
          this->NumberOfAnimations++;
          this->AnimationNames.emplace_back(meshName);
        }
        else if (meshName != frameName)
        {
          animationId++;
          frameName = meshName;
          this->NumberOfAnimations++;
          this->AnimationNames.emplace_back(meshName);
        }
        float time = selectedFrame.time == nullptr ? frameNum * (1.0 / this->FrameRate)
                                                   : selectedFrame.time[groupFrameNum];
        this->AnimationIds.emplace_back(std::make_pair(animationId, time));
      }
    }

    // Add interpolated frames
    // Linear interpolation between frames in the same animation.
    for (std::size_t i = 0; i < this->Mesh.size() - 1; i++)
    {
      if (this->AnimationIds[i + 1].first != this->AnimationIds[i].first)
      {
        continue;
      }
      else
      {
        vtkNew<vtkPoints> vertices;
        vertices->Allocate(header->numTriangles * 3);
        for (int j = 0; j < header->numTriangles * 3; j++)
        {
          double* v_0 = this->Mesh[i]->GetPoint(j);
          double* v_1 = this->Mesh[i + 1]->GetPoint(j);
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
        // Inserts frame between i and i+1
        this->Mesh.insert(this->Mesh.begin() + i, mesh);
        std::pair<int, float> pair = std::make_pair(this->AnimationIds[i].first,
          (this->AnimationIds[i].second + this->AnimationIds[i + 1].second) / 2);
        // Increments i to avoid infinite loop
        this->AnimationIds.insert(this->AnimationIds.begin() + i++, pair); 
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
    this->Texture = this->CreateTexture(
      buffer, offset, header->skinWidth, header->skinHeight, header->numSkins, 0);

    // Set polyData
    this->CreateMesh(buffer, offset, header);

    return true;
  }

  //----------------------------------------------------------------------------
  void UpdateTimeStep(double timeValue)
  {
    int frameIndex = (int)floor(FrameRate * abs(timeValue)) % (int)this->ActiveFrames.size();
    int currentFrame = this->ActiveFrames[frameIndex];
    this->Mapper->SetInputData(this->Mesh[currentFrame]);
  }

  //----------------------------------------------------------------------------
  void EnableAnimation(vtkIdType animationIndex)
  {
    int firstFrameIndex = std::distance(this->AnimationIds.begin(),
      std::find_if(this->AnimationIds.begin(), this->AnimationIds.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first == animationIndex; }));
    int lastFrameIndex = std::distance(this->AnimationIds.begin(),
      std::find_if(this->AnimationIds.begin(), this->AnimationIds.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first > animationIndex; }));
    firstFrameIndex = firstFrameIndex <= (int)this->AnimationIds.size() ? firstFrameIndex : 0;
    lastFrameIndex =
      lastFrameIndex <= (int)this->AnimationIds.size() ? lastFrameIndex - 1 : lastFrameIndex;
    for (int i = firstFrameIndex; i <= lastFrameIndex; i++)
    {
      this->ActiveFrames.emplace_back(i);
    }
    this->ActiveAnimationIds.emplace_back(animationIndex);
  }

  //----------------------------------------------------------------------------
  void DisableAnimation(vtkIdType animationIndex)
  {
    int firstFrameIndex = std::distance(this->AnimationIds.begin(),
      std::find_if(this->AnimationIds.begin(), this->AnimationIds.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first == animationIndex; }));
    int lastFrameIndex = std::distance(this->AnimationIds.begin(),
      std::find_if(this->AnimationIds.begin(), this->AnimationIds.end(),
        [animationIndex](const std::pair<int, float> pair)
        { return pair.first > animationIndex; }));
    firstFrameIndex = firstFrameIndex <= (int)this->AnimationIds.size() ? firstFrameIndex : 0;
    lastFrameIndex =
      lastFrameIndex <= (int)this->AnimationIds.size() ? lastFrameIndex - 1 : lastFrameIndex;
    for (int i = firstFrameIndex; i <= lastFrameIndex; i++)
    {
      this->ActiveFrames.erase(
        std::remove(this->ActiveFrames.begin(), this->ActiveFrames.end(), i), this->ActiveFrames.end());
    }
    this->ActiveAnimationIds.erase(
      std::remove(this->ActiveAnimationIds.begin(), this->ActiveAnimationIds.end(), animationIndex),
      this->ActiveAnimationIds.end());
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
    this->Mapper = mapper;
  }

  //----------------------------------------------------------------------------
  void SetFrameRate(double frameRate)
  {
    this->FrameRate = frameRate;
  }

  //----------------------------------------------------------------------------
  void GetTimeRange(vtkIdType vtkNotUsed(animationIndex), double timeRange[2])
  {
    timeRange[0] = 0.0;
    timeRange[1] = (1.0 / this->FrameRate) * this->AnimationIds.size();
  }

  vtkF3DQuakeMDLImporter* Parent;
  std::string Description;
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  std::vector<vtkSmartPointer<vtkPolyData>> Mesh;
  std::vector<std::pair<int, float>> AnimationIds;
  std::vector<std::string> AnimationNames;
  std::vector<int> ActiveFrames;
  std::vector<int> ActiveAnimationIds;
  int NumberOfAnimations = 0;
  double FrameRate = 60.0;
  vtkSmartPointer<vtkTexture> Texture;
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
  if (animationIndex < (int)this->Internals->AnimationNames.size())
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
  return std::count(this->Internals->ActiveAnimationIds.begin(),
           this->Internals->ActiveAnimationIds.end(), animationIndex) > 0;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(vtkIdType animationIndex, double frameRate,
  int& nbTimeSteps, double timeRange[2], vtkDoubleArray* vtkNotUsed(timeSteps))
{
  this->Internals->SetFrameRate(frameRate);
  this->Internals->GetTimeRange(animationIndex, timeRange);
  nbTimeSteps = (int)this->Internals->ActiveFrames.size();
  return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfCameras()
{
  return 0;
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetCameraName(vtkIdType vtkNotUsed(camIndex))
{
  return "Camera";
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::ImportCameras(vtkRenderer* vtkNotUsed(renderer))
{
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::ImportLights(vtkRenderer* vtkNotUsed(renderer))
{
}
