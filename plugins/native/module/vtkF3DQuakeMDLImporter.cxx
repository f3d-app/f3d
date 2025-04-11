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
    const int* type;   // 0 = simple frame, !0 = group frames
    const int* nb;     // "number", size of *time and *frames arrays, optional
                       // const mdl_vertex_t* min;         // min pos in all simple frames, unused
                       // const mdl_vertex_t* max;         // max pos in all simple frames, unused
    const float* time; // time duration for each frame, optional
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
    vtkNew<vtkTexture> texture;
    texture->InterpolateOn();

    // Read textures.
    std::vector<mixed_pointer_array> skins = std::vector<mixed_pointer_array>(nbSkins);
    for (int i = 0; i < nbSkins; i++)
    {
      skins[i].group = *reinterpret_cast<const int*>(buffer.data() + offset);
      if (skins[i].group == 0)
      {
        skins[i].skin = buffer.data() + sizeof(int32_t) + offset;
        offset += sizeof(int32_t) + skinWidth * skinHeight * sizeof(int8_t);
      }
      else
      {
        int nb = *reinterpret_cast<const int*>(buffer.data() + offset + 4);
        skins[i].skin = buffer.data() + 4 + nb * 4 + offset;
        offset +=
          sizeof(int32_t) + nb * sizeof(int32_t) + nb * skinWidth * skinHeight * sizeof(int8_t);
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
  vtkSmartPointer<vtkPolyData> CreateMeshForSimpleFrame(const mdl_simpleframe_t* frame,
    const mdl_header_t* header, const mdl_triangle_t* triangles, vtkCellArray* cells,
    vtkFloatArray* textureCoordinates)
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
          F3DMDLNormalVectors[normalIndex][1] / 255.0, F3DMDLNormalVectors[normalIndex][2] / 255.0);
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
    constexpr int mdl_simpleframe_t_fixed_size =
      2 * sizeof(mdl_vertex_t) + 16 * sizeof(int8_t); // Size of bboxmin, bboxmax and name.

    // Read Texture Coordinates
    const mdl_texcoord_t* texcoords =
      reinterpret_cast<const mdl_texcoord_t*>(buffer.data() + offset);
    offset += sizeof(mdl_texcoord_t) * header->numVertices;

    // Read Triangles
    const mdl_triangle_t* triangles =
      reinterpret_cast<const mdl_triangle_t*>(buffer.data() + offset);
    offset += sizeof(mdl_triangle_t) * header->numTriangles;

    // Read frames
    int frameType = -1; // To check for mixed frame types
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
        vtkErrorWithObjectMacro(
          this->Parent, "Combined simple frame and group frame are not supported, aborting.");
        return false;
      }
      if (*framePtr[i].type == 0)
      {
        framePtr[i].nb = nullptr;
        framePtr[i].time = nullptr;
        framePtr[i].frames =
          reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + sizeof(int32_t) + offset);

        // Size of a frame is mdl_simpleframe_t_fixed_size + mdl_vertex_t * numVertices, +
        // sizeof(int)
        offset += sizeof(int32_t) + mdl_simpleframe_t_fixed_size +
          sizeof(mdl_vertex_t) * header->numVertices;
      }
      else
      {
        framePtr[i].nb = reinterpret_cast<const int*>(buffer.data() + sizeof(int32_t) + offset);
        // Skips parameters min and max.
        framePtr[i].time = reinterpret_cast<const float*>(reinterpret_cast<const void*>(
          buffer.data() + 2 * sizeof(int32_t) + 2 * sizeof(mdl_vertex_t) + offset));
        // Points to the first frame, 4 * nbFrames for the float array
        framePtr[i].frames =
          reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + 2 * sizeof(int32_t) +
            2 * sizeof(mdl_vertex_t) + (*framePtr[i].nb) * sizeof(float) + offset);
        offset +=
          2 * sizeof(int32_t) + 2 * sizeof(mdl_vertex_t) + (*framePtr[i].nb) * sizeof(float);
        frameOffsets.emplace_back(std::vector<int>());

        for (int j = 0; j < *framePtr[i].nb; j++)
        {
          // Offset for each frame
          frameOffsets[i].emplace_back(offset);
          offset += mdl_simpleframe_t_fixed_size + sizeof(mdl_vertex_t) * header->numVertices;
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

    // A lambda to check frame name for standard naming scheme
    // and recover animation name
    auto extract_animation_name = [](const std::string& frameName)
    {
      std::string::size_type sz;
      sz = frameName.find_first_of("0123456789");
      if (sz == std::string::npos)
      {
        return std::tuple<bool, std::string>(false, "");
      }
      return std::tuple<bool, std::string>(true, frameName.substr(0, sz));
    };

    // A lambda to add an empty named animation, return the index to it
    auto emplace_empty_animation = [&](const std::string& animName)
    {
      this->AnimationNames.emplace_back(animName);
      this->AnimationTimes.emplace_back(std::vector<double>());
      this->AnimationFrames.emplace_back(std::vector<vtkSmartPointer<vtkPolyData>>());
      return this->AnimationNames.size() - 1;
    };

    // Create frames
    if (frameType == 0)
    {
      bool standardNamingScheme = true;
      for (int frameNum = 0; frameNum < header->numFrames; frameNum++)
      {
        // Recover pointer to the single frame
        const mdl_simpleframe_t* frame = framePtr[frameNum].frames;

        // Extract animation name from frame name and recover animation index accordingly
        size_t animationIdx = this->AnimationNames.size() - 1;
        if (standardNamingScheme)
        {
          // Check if frame name respect standard naming scheme for simple frames
          // eg: stand1, stand2, stand3, run1, run2, run3
          // XXX: Multi animation files with standard naming scheme are not tested, here is (a non
          // free) one for manual testing if needed:
          // https://tomeofpreach.wordpress.com/2012/12/24/shambler-dance/
          // XXX: This code assume frames are provided in order and does not check the numbering
          auto [standard, animationName] = extract_animation_name(frame->name);
          if (!standard)
          {
            // If one frame is misnamed, give up and put every remaining frame in the same animation
            standardNamingScheme = false;
            animationIdx = emplace_empty_animation("Animation");
            vtkWarningWithObjectMacro(this->Parent,
              "Frame name does not respect standard naming scheme: " + std::string(frame->name) +
                ", animation may be incorrect");
          }
          else
          {
            auto it =
              std::find(this->AnimationNames.begin(), this->AnimationNames.end(), animationName);
            if (it != this->AnimationNames.end())
            {
              animationIdx = std::distance(this->AnimationNames.begin(), it);
            }
            else
            {
              animationIdx = emplace_empty_animation(animationName);
            }
          }
        }

        // Handle animation times
        std::vector<double>& times = this->AnimationTimes[animationIdx];
        if (times.empty())
        {
          times.emplace_back(0.0);
        }
        else
        {
          // Simple frames are 10 fps
          times.emplace_back(times.back() + 0.1);
        }

        // Create the animation frame
        vtkSmartPointer<vtkPolyData> mesh =
          this->CreateMeshForSimpleFrame(frame, header, triangles, cells, textureCoordinates);
        this->AnimationFrames[animationIdx].emplace_back(mesh);
      }
    }
    else
    {
      for (int frameNum = 0; frameNum < header->numFrames; frameNum++)
      {
        plugin_frame_pointer frameGroup = framePtr[frameNum];

        std::string animationName;
        std::vector<double> times;
        std::vector<vtkSmartPointer<vtkPolyData>> meshes;

        // Iterate over each frame in the group
        for (int groupFrameNum = 0; groupFrameNum < *frameGroup.nb; groupFrameNum++)
        {
          // Recover the frame using the offsets because the struct does not store this pointer
          const mdl_simpleframe_t* frame = reinterpret_cast<const mdl_simpleframe_t*>(
            buffer.data() + frameOffsets[frameNum][groupFrameNum]);

          // Assume all frames are named the with standard naming scheme
          if (animationName.empty())
          {
            auto [standard, localAnimationName] = extract_animation_name(frame->name);
            if (standard)
            {
              animationName = localAnimationName;
            }
            else
            {
              vtkWarningWithObjectMacro(this->Parent,
                "Frame name does not respect standard naming scheme: " + std::string(frame->name) +
                  ", animation may be misnamed");
              animationName = frame->name;
            }
          }

          // Recover time for this frame from the dedicated table
          times.emplace_back(frameGroup.time[groupFrameNum]);

          // Recover mesh for this frame
          meshes.emplace_back(
            this->CreateMeshForSimpleFrame(frame, header, triangles, cells, textureCoordinates));
        }
        this->AnimationNames.emplace_back(animationName);
        this->AnimationTimes.emplace_back(times);
        this->AnimationFrames.emplace_back(meshes);
      }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  bool ReadScene(const std::string& filePath)
  {
    std::ifstream inputStream(filePath, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});

    // Read header
    int offset = 0;
    const mdl_header_t* header = reinterpret_cast<const mdl_header_t*>(buffer.data());
    offset += sizeof(mdl_header_t);

    // Create textures
    this->Texture = this->CreateTexture(
      buffer, offset, header->skinWidth, header->skinHeight, header->numSkins, 0);

    // Create animation frames
    bool ret = this->CreateMesh(buffer, offset, header);
    if (this->AnimationFrames.empty() || this->AnimationFrames.front().empty())
    {
      vtkErrorWithObjectMacro(
        this->Parent, "No frame read, there is nothing to display in this file.");
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
  const std::vector<double>& times =
    this->Internals->AnimationTimes[this->Internals->ActiveAnimation];

  // Find frameIndex for the provided timeValue so that t0 <= timeValue < t1

  // First time >= value
  const auto found = std::lower_bound(times.begin(), times.end(), timeValue);
  // If none, select last, if found, select distance
  const size_t i = found == times.end() ? times.size() - 1 : std::distance(times.begin(), found);
  // If found time > timeValue, the the previous one
  const size_t frameIndex = *found > timeValue && i > 0 ? i - 1 : i;

  this->Internals->Mapper->SetInputData(
    this->Internals->AnimationFrames[this->Internals->ActiveAnimation][frameIndex]);
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
  assert(animationIndex < this->Internals->AnimationNames.size());
  assert(animationIndex >= 0);
  return this->Internals->AnimationNames[animationIndex];
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::EnableAnimation(vtkIdType animationIndex)
{
  assert(animationIndex < this->Internals->AnimationNames.size());
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
  assert(animationIndex < this->Internals->AnimationNames.size());
  assert(animationIndex >= 0);
  return this->Internals->ActiveAnimation == animationIndex;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& vtkNotUsed(nbTimeSteps), double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  assert(animationIndex < this->Internals->AnimationNames.size());
  assert(animationIndex >= 0);

  const std::vector<double>& times = this->Internals->AnimationTimes[animationIndex];
  // F3D do not care about timesteps, only set time range
  timeRange[0] = times.front();
  timeRange[1] = times.back();
  return true;
}
