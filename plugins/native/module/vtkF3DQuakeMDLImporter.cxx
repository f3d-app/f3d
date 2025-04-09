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
struct vtkF3DQuakeMDLImporter::vtkInternals
{
  //----------------------------------------------------------------------------
  // Different structs used for parsing the binary data

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

  // Simple vertex and normal
  struct mdl_vertex_t
  {
    unsigned char xyz[3];
    unsigned char normalIndex;
  };

  // Triangles
  struct mdl_triangle_t
  {
    int facesFront; // 0 = backface, 1 = frontface
    int vertex[3];  // vertex indices
  };

  // Texture coordinates
  struct mdl_texcoord_t
  {
    int onseam;
    int coord_s;
    int coord_t;
  };

  // Texture
  struct mixed_pointer_array
  {
    int group;
    const unsigned char* skin;
  };

  // Simple frame
  struct mdl_simpleframe_t // 24 + nbVertices bytes
  {
    mdl_vertex_t bboxmin;     // bounding box min
    mdl_vertex_t bboxmax;     // bounding box max
    char name[16];            // frame name
    mdl_vertex_t verts[1024]; // vertex list of the frame, maximum capacity is 1024
  };

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

  //----------------------------------------------------------------------------
  explicit vtkInternals(vtkF3DQuakeMDLImporter* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkTexture> CreateTexture(const std::vector<unsigned char>& buffer, int& offset,
    int skinWidth, int skinHeight, int nbSkins, int selectedSkinIndex)
  {
    constexpr int char_size = sizeof(int8_t);
    constexpr int int_size = sizeof(int32_t);
    vtkNew<vtkTexture> texture;
    texture->InterpolateOn();

    // Read textures.
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
        std::copy(F3DMDLDefaultColorMap[index], F3DMDLDefaultColorMap[index] + 3, ptr);
      }
    }
    texture->SetInputData(img);
    return texture;
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> CreateMeshForSimpleFrame(const mdl_simpleframe_t* frame, const mdl_header_t* header, const mdl_triangle_t* triangles, vtkCellArray* cells, vtkFloatArray* textureCoordinates)
  {
    vtkNew<vtkPoints> vertices;
    vertices->Allocate(header->numTriangles * 3);

    vtkNew<vtkFloatArray> normals;
    normals->SetNumberOfComponents(3);
    normals->Allocate(header->numTriangles * 3 * 3);

    for (int i = 0; i < header->numTriangles; i++)
    {
      vtkIdType vertexNum[3];
      for (int j = 0; j < 3; j++)
      {
        // Recover XYZ
        vertexNum[j] = triangles[i].vertex[j];
        double xyz[3] = { static_cast<double>(frame->verts[vertexNum[j]].xyz[0]),
          static_cast<double>(frame->verts[vertexNum[j]].xyz[1]),
          static_cast<double>(frame->verts[vertexNum[j]].xyz[2]) };

        // Calculate real vertex position
        for (int k = 0; k < 3; k++)
        {
          xyz[k] = xyz[k] * header->scale[k] + header->translation[k];
        }
        vertices->InsertPoint(i * 3 + j, xyz);

        // Normal vector
        int normalIndex = frame->verts[vertexNum[j]].normalIndex;
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
    return mesh;
  }

  //----------------------------------------------------------------------------
  bool CreateMesh(const std::vector<unsigned char>& buffer, int offset, const mdl_header_t* header)
  {
    // TODO replace this by actual size_of ?
    constexpr char char_size = sizeof(int8_t); // Size of char in file
    constexpr int int_size = sizeof(int32_t);  // Size of int in file
    constexpr int float_size = sizeof(float);  // Size of float in file, 4 bytes
    constexpr int mdl_texcoord_t_size = 3 * int_size; // Size of struct
    constexpr int mdl_triangle_t_size = 4 * int_size; // Size of struct
    constexpr int mdl_vertex_t_size = 4;
    constexpr int mdl_simpleframe_t_fixed_size =
      2 * mdl_vertex_t_size + 16 * char_size; // Size of bboxmin, bboxmax and name.

    // Read Texture Coordinates
    const mdl_texcoord_t* texcoords =
      reinterpret_cast<const mdl_texcoord_t*>(buffer.data() + offset);
    offset += mdl_texcoord_t_size * header->numVertices; // Size of mdl_texcoord_t array

    // Read Triangles
    const mdl_triangle_t* triangles =
      reinterpret_cast<const mdl_triangle_t*>(buffer.data() + offset);
    offset += mdl_triangle_t_size * header->numTriangles; // Size of mdl_triangle_t array

    // Read frames
    int frameType = -1; // To check for mixed types
    std::vector<plugin_frame_pointer> framePtr =
      std::vector<plugin_frame_pointer>(header->numFrames);
    std::vector<std::vector<int>> frameOffsets = std::vector<std::vector<int>>();
    for (int i = 0; i < header->numFrames; i++)
    {
      framePtr[i].type = reinterpret_cast<const int*>(buffer.data() + offset);
      if (frameType == -1)
      {
        frameType = *framePtr[i].type;
      }
      if (frameType != *framePtr[i].type)
      {
        vtkErrorWithObjectMacro(this->Parent, "Combined simple frame and group frame are not supported, aborting.");
        return false;
      }
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
    for (int i = 0; i < header->numTriangles; i++)
    {
      for (int vertex : triangles[i].vertex)
      {
        float coord_s = texcoords[vertex].coord_s;
        float coord_t = texcoords[vertex].coord_t;
        if (!triangles[i].facesFront && texcoords[vertex].onseam)
        {
          coord_s = coord_s + header->skinWidth * 0.5f; // Backface
        }

        // Scale s and t to range from 0.0 to 1.0
        coord_s = (coord_s + 0.5) / header->skinWidth;
        coord_t = (coord_t + 0.5) / header->skinHeight;
        float coords_st[2] = { coord_s, coord_t };
        textureCoordinates->InsertNextTuple(coords_st);
      }
      vtkIdType triangle[3] = { i * 3, i * 3 + 1, i * 3 + 2 };
      cells->InsertNextCell(3, triangle);
    }

    // Create frames
    if (frameType == 0)
    {
      // Only simple frames, create a single animation
      this->AnimationNames.emplace_back("Animation");
      this->AnimationTimes.emplace_back(std::vector<double>());
      this->AnimationFrames.emplace_back(std::vector<vtkSmartPointer<vtkPolyData>>());
    }

    for (int frameNum = 0; frameNum < header->numFrames; frameNum++)
    {
      plugin_frame_pointer selectedFrame = framePtr[frameNum];
      if (frameType == 0)
      {
        // Handle animation times
        std::vector<double>& times = this->AnimationTimes[0];
        if (times.empty())
        {
          times.emplace_back(0.0);
        }
        else
        {
          // Simple frames are 10 fps
          times.emplace_back(times.back() + 0.1);
        }

        // Recover pointer to the single frame
        const mdl_simpleframe_t* frame = selectedFrame.frames;
        vtkSmartPointer<vtkPolyData> mesh = this->CreateMeshForSimpleFrame(frame, header, triangles, cells, textureCoordinates);
        this->AnimationFrames[0].emplace_back(mesh);
      }
      else
      {
        // TODO proper animation names
        this->AnimationNames.emplace_back("Animation");

        std::vector<double> times;
        std::vector<vtkSmartPointer<vtkPolyData>> meshes;

        // Iterate over each frame in the group
        for (int groupFrameNum = 0; groupFrameNum < *selectedFrame.nb; groupFrameNum++)
        {
          // Recover the frame using the offsets because the struct doesnt store this pointer
          const mdl_simpleframe_t* frame = reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + frameOffsets[frameNum][groupFrameNum]);

          // Recover time for this frame from the dedicated table
          times.emplace_back(selectedFrame.time[groupFrameNum]);

          // Recover mesh for this frame
          meshes.emplace_back(this->CreateMeshForSimpleFrame(frame, header, triangles, cells, textureCoordinates));
        }
        this->AnimationTimes.emplace_back(times);
        this->AnimationFrames.emplace_back(meshes);
      }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  bool ReadScene(const std::string& filePath)
  {
    // TODO use size_of ?
    constexpr int mdl_header_t_size = 84; // Size of the header struct in the file.

    std::ifstream inputStream(filePath, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});

    // Read header
    int offset = 0;
    const mdl_header_t* header = reinterpret_cast<const mdl_header_t*>(buffer.data());
    offset += mdl_header_t_size;

    // Create textures
    this->Texture = this->CreateTexture(
      buffer, offset, header->skinWidth, header->skinHeight, header->numSkins, 0);

    // Create animation frames
    bool ret = this->CreateMesh(buffer, offset, header);
    if (this->AnimationFrames.empty() || this->AnimationFrames.front().empty())
    {
      vtkErrorWithObjectMacro(this->Parent, "No frame read, there is nothing to display in this file.");
      ret = false;
    }

    return ret;
  }

  //----------------------------------------------------------------------------
  void ImportActors(vtkRenderer* renderer)
  {
    vtkNew<vtkActor> actor;
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(this->AnimationFrames[0][0]);
    actor->SetMapper(mapper);
    actor->SetTexture(this->Texture);
    renderer->AddActor(actor);
    renderer->SetBackground(0, 0, 0);
    this->Mapper = mapper;
  }

  //----------------------------------------------------------------------------
  vtkF3DQuakeMDLImporter* Parent;
  std::string Description;
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  vtkSmartPointer<vtkTexture> Texture;

  std::vector<std::string> AnimationNames;
  std::vector<std::vector<double>> AnimationTimes;
  std::vector<std::vector<vtkSmartPointer<vtkPolyData>>> AnimationFrames;

  vtkIdType ActiveAnimation = 0;
};

//----------------------------------------------------------------------------
vtkF3DQuakeMDLImporter::vtkF3DQuakeMDLImporter()
  : Internals(new vtkF3DQuakeMDLImporter::vtkInternals(this))
{
}

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
bool vtkF3DQuakeMDLImporter::UpdateAtTimeValue(double timeValue)
{
  const std::vector<double>& times = this->Internals->AnimationTimes[this->Internals->ActiveAnimation];

  // TODO find_if ?
  size_t frameIndex = 0;
  if (timeValue <= times.front())
  {
    frameIndex = 0;
  }
  else if (timeValue >= times.back())
  {
    frameIndex = times.size() - 1;
  }
  else
  {
    for (;times.size() - 1; frameIndex++)
    {
      if (times[frameIndex] <= timeValue && timeValue < times[frameIndex + 1])
      {
        break;
      }
    }
  }

  this->Internals->Mapper->SetInputData(this->Internals->AnimationFrames[this->Internals->ActiveAnimation][frameIndex]);
  return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfAnimations()
{
  return this->Internals->AnimationNames.size();
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetAnimationName(vtkIdType animationIndex)
{
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size()));
  assert(animationIndex >= 0);
  return this->Internals->AnimationNames[animationIndex];
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::EnableAnimation(vtkIdType animationIndex)
{
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size()));
  assert(animationIndex >= 0);
  this->Internals->ActiveAnimation = animationIndex;
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::DisableAnimation(vtkIdType vtkNotUsed(animationIndex))
{
  this->Internals->ActiveAnimation = -1;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  assert(animationIndex < this->GetNumberOfAnimations());
  assert(animationIndex >= 0);
  return this->Internals->ActiveAnimation == animationIndex;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& vtkNotUsed(nbTimeSteps), double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  assert(animationIndex < this->GetNumberOfAnimations());
  assert(animationIndex >= 0);

  const std::vector<double>& times = this->Internals->AnimationTimes[animationIndex];
  // F3D do not care about timesteps, only set time range
  timeRange[0] = times.front();
  timeRange[1] = times.back();
  return true;
}
