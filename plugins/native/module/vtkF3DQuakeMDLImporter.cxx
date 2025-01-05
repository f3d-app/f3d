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
    constexpr int char_size = 1;
    constexpr int int_size = 4;
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
        skins[i].skin = buffer.data() + int_size + offset;
        offset += int_size + skinWidth * skinHeight * char_size;
      }
      else
      {
        int nb = *reinterpret_cast<const int*>(buffer.data() + offset + 4);
        skins[i].skin = buffer.data() + 4 + nb * 4 + offset;
        offset += int_size + nb * int_size + nb * skinWidth * skinHeight * char_size;
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
  void CreateMesh(const std::vector<unsigned char>& buffer, int offset, const mdl_header_t* header)
  {
    constexpr char char_size = 1; // Size of char in file
    constexpr int int_size = 4;   // Size of int in file
    constexpr int float_size = 4; // Size of float in file
    // Read texture coordinates
    struct mdl_texcoord_t
    {
      int onseam;
      int coord_s;
      int coord_t;
    };
    constexpr int mdl_texcoord_t_size = 3 * int_size; // Size of struct
    const mdl_texcoord_t* texcoords =
      reinterpret_cast<const mdl_texcoord_t*>(buffer.data() + offset);
    offset += mdl_texcoord_t_size * header->numVertices; // Size of mdl_texcoord_t array

    // Read triangles
    struct mdl_triangle_t
    {
      int facesFront; // 0 = backface, 1 = frontface
      int vertex[3];  // vertex indices
    };
    constexpr int mdl_triangle_t_size = 4 * int_size; // Size of struct
    const mdl_triangle_t* triangles =
      reinterpret_cast<const mdl_triangle_t*>(buffer.data() + offset);
    offset += mdl_triangle_t_size * header->numTriangles; // Size of mdl_triangle_t array

    // Simple vertex and normal
    struct mdl_vertex_t
    {
      unsigned char xyz[3];
      unsigned char normalIndex;
    };
    constexpr int mdl_vertex_t_size = 4;
    // Simple frame
    struct mdl_simpleframe_t // 24 + nbVertices bytes
    {
      mdl_vertex_t bboxmin;     // bounding box min
      mdl_vertex_t bboxmax;     // bounding box max
      char name[16];            // frame name
      mdl_vertex_t verts[1024]; // vertex list of the frame, maximum capacity is 1024
    };
    constexpr int mdl_simpleframe_t_fixed_size =
      2 * mdl_vertex_t_size + 16 * char_size; // Size of bboxmin, bboxmax and name.
    // Struct containing frame type and data
    // Used to read mdl_frame_t and mdl_groupframe_t
    struct plugin_frame_pointer
    {
      const int* type; // 0 = simple frame, !0 = group frames
      const int* nb;   // "number", size of *time and *frames arrays, optional
                       // const mdl_vertex_t* min;         // min pos in all simple frames, optional
                       // const mdl_vertex_t* max;         // max pos in all simple frames, optional
      const float* time;               // time duration for each frame, optional
      const mdl_simpleframe_t* frames; // simple frame list
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
        // Size of a frame is 24 + 4 * numVertices, + 4 bytes for the int
        offset += int_size + mdl_simpleframe_t_fixed_size + mdl_vertex_t_size * header->numVertices;
      }
      else
      {
        framePtr[i].nb = reinterpret_cast<const int*>(buffer.data() + int_size + offset);
        // Skips parameters min and max.
        framePtr[i].time = reinterpret_cast<const float*>(reinterpret_cast<const void*>(
          buffer.data() + 2 * int_size + 2 * mdl_vertex_t_size + offset));
        // Points to the first frame, 4 * nbFrames for the float array
        framePtr[i].frames = reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() +
          2 * int_size + 2 * mdl_vertex_t_size + (*framePtr[i].nb) * float_size + offset);
        offset += 2 * int_size + 2 * mdl_vertex_t_size + (*framePtr[i].nb) * float_size;
        frameOffsets.emplace_back(std::vector<int>());
        for (int j = 0; j < *framePtr[i].nb; j++)
        {
          // Offset for each frame
          frameOffsets[i].emplace_back(offset);
          offset += mdl_simpleframe_t_fixed_size + mdl_vertex_t_size * header->numVertices;
        }
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
      float coord_s;
      float coord_t;
    };
    std::vector<plugin_texture_coords> coords =
      std::vector<plugin_texture_coords>(header->numTriangles * 3);
    for (int i = 0; i < header->numTriangles; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        float coord_s = texcoords[triangles[i].vertex[j]].coord_s;
        float coord_t = texcoords[triangles[i].vertex[j]].coord_t;
        if (!triangles[i].facesFront && texcoords[triangles[i].vertex[j]].onseam)
        {
          coord_s = coord_s + header->skinWidth * 0.5f; // Backface
        }
        // Scale s and t to range from 0.0 to 1.0
        coord_s = (coord_s + 0.5) / header->skinWidth;
        coord_t = (coord_t + 0.5) / header->skinHeight;
        coords[3 * i + j].coord_s = coord_s;
        coords[3 * i + j].coord_t = coord_t;
        float coords_st[2] = { coord_s, coord_t };
        textureCoordinates->InsertNextTuple(coords_st);
      }
      vtkIdType triangle[3] = { i * 3, i * 3 + 1, i * 3 + 2 };
      cells->InsertNextCell(3, triangle);
    }

    // Draw frames
    std::string frameName;
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
          ? reinterpret_cast<const mdl_simpleframe_t*>(
              buffer.data() + frameOffsets[frameNum][groupFrameNum])
          : selectedFrame.frames;
        for (int i = 0; i < header->numTriangles; i++)
        {
          vtkIdType vertexNum[3];
          for (int j = 0; j < 3; j++)
          {
            vertexNum[j] = triangles[i].vertex[j];
            double xyz[3] = { static_cast<double>(selectedSimpleFrame->verts[vertexNum[j]].xyz[0]),
              static_cast<double>(selectedSimpleFrame->verts[vertexNum[j]].xyz[1]),
              static_cast<double>(selectedSimpleFrame->verts[vertexNum[j]].xyz[2]) };
            // Calculate real vertex position
            for (int k = 0; k < 3; k++)
            {
              xyz[k] = xyz[k] * header->scale[k] + header->translation[k];
            }
            vertices->InsertPoint(i * 3 + j, xyz);
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
          double* vertex0 = this->Mesh[i]->GetPoint(j);
          double* vertex1 = this->Mesh[i + 1]->GetPoint(j);
          double interp[3];
          interp[0] = vertex0[0] + 0.5 * (vertex1[0] - vertex0[0]);
          interp[1] = vertex0[1] + 0.5 * (vertex1[1] - vertex0[1]);
          interp[2] = vertex0[2] + 0.5 * (vertex1[2] - vertex0[2]);
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
    int offset = 0;
    const mdl_header_t* header = reinterpret_cast<const mdl_header_t*>(buffer.data());
    constexpr int mdl_header_t_size = 84; // Size of the header struct in the file.
    offset += mdl_header_t_size;
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
    int frameIndex = static_cast<int>(floor(FrameRate * abs(timeValue))) %
      static_cast<int>(this->ActiveFrames.size());
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
    firstFrameIndex =
      firstFrameIndex <= static_cast<int>(this->AnimationIds.size()) ? firstFrameIndex : 0;
    lastFrameIndex = lastFrameIndex <= static_cast<int>(this->AnimationIds.size())
      ? lastFrameIndex - 1
      : lastFrameIndex;
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
    firstFrameIndex =
      firstFrameIndex <= static_cast<int>(this->AnimationIds.size()) ? firstFrameIndex : 0;
    lastFrameIndex = lastFrameIndex <= static_cast<int>(this->AnimationIds.size())
      ? lastFrameIndex - 1
      : lastFrameIndex;
    for (int i = firstFrameIndex; i <= lastFrameIndex; i++)
    {
      this->ActiveFrames.erase(std::remove(this->ActiveFrames.begin(), this->ActiveFrames.end(), i),
        this->ActiveFrames.end());
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
bool vtkF3DQuakeMDLImporter::UpdateAtTimeValue(double timeValue)
{
  this->Internals->UpdateTimeStep(timeValue);
  return timeValue <= this->Internals->FrameRate * this->Internals->AnimationIds.size();
}


//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfAnimations()
{
  return this->Internals->NumberOfAnimations;
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetAnimationName(vtkIdType animationIndex)
{
  if (animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size()))
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
  nbTimeSteps = static_cast<int>(this->Internals->ActiveFrames.size());
  return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfCameras()
{
  return 0;
}
