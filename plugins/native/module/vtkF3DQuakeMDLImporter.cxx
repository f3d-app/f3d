#include "vtkF3DQuakeMDLImporter.h"
#include "vtkF3DQuakeMDLImporterConstants.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkOpenGLTexture.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
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

  enum FRAME_TYPE
  {
    SINGLE_FRAME = 0,
    GROUP_FRAME,
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
    int skinWidth, int skinHeight, unsigned int nbSkins, unsigned int skinIndex)
  {
    vtkNew<vtkTexture> texture;
    texture->SetColorModeToDirectScalars();
    texture->UseSRGBColorSpaceOn();

    // Read textures.
    std::vector<mixed_pointer_array> skins = std::vector<mixed_pointer_array>(nbSkins);
    for (unsigned int i = 0; i < nbSkins; i++)
    {
      skins[i].group = *reinterpret_cast<const int*>(buffer.data() + offset);
      if (skins[i].group == 0)
      {
        skins[i].skin = buffer.data() + sizeof(int32_t) + offset;
        offset += sizeof(int32_t) + skinWidth * skinHeight * sizeof(int8_t);
      }
      else
      {
        // XXX: groupskin not supported yet
        vtkErrorWithObjectMacro(this->Parent, "Groupskin are not supported, aborting.");
        return nullptr;
      }
    }

    // Copy to imageData
    vtkNew<vtkImageData> img;
    img->SetDimensions(skinWidth, skinHeight, 1);
    img->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

    if (skinIndex >= nbSkins)
    {
      skinIndex = 0;
      vtkWarningWithObjectMacro(
         this->Parent, "QuakeMDL.skin_index is out of bounds. Defauling to 0.");
    }
    const unsigned char* selectedSkin = skins[skinIndex].skin;
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
    normals->SetNumberOfTuples(header->numTriangles * 3);

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
        normals->SetTypedTuple(i * 3 + j, F3DMDLNormalVectors[normalIndex]);
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
    std::vector<plugin_frame_pointer> framePtr =
      std::vector<plugin_frame_pointer>(header->numFrames);
    std::vector<std::vector<int>> frameOffsets = std::vector<std::vector<int>>();
    for (int i = 0; i < header->numFrames; i++)
    {
      framePtr[i].type = reinterpret_cast<const int*>(buffer.data() + offset);
      if (*framePtr[i].type == SINGLE_FRAME)
      {
        framePtr[i].nb = nullptr;
        framePtr[i].time = nullptr;
        framePtr[i].frames =
          reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + sizeof(int32_t) + offset);

        // Size of a frame is mdl_simpleframe_t_fixed_size + mdl_vertex_t * numVertices, +
        // sizeof(int)
        offset += sizeof(int32_t) + mdl_simpleframe_t_fixed_size +
          sizeof(mdl_vertex_t) * header->numVertices;

        // Always emplace in case of mixed single frame and group frame
        frameOffsets.emplace_back(std::vector<int>());
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
      // triangle winding order is inverted in Quake MDL
      vtkIdType triangle[3] = { i * 3, i * 3 + 2, i * 3 + 1 };
      cells->InsertNextCell(3, triangle);
    }

    // Extract animation name from frame name and recover animation index accordingly
    // Check if frame name respect standard naming scheme for single frames
    // eg: stand1, stand2, stand3, run1, run2, run3
    // XXX: This code assume frames are provided in order and does not check the numbering
    // If not, just return the frame name
    auto extract_animation_name = [&](const std::string& frameName)
    {
      std::string::size_type sz;
      sz = frameName.find_first_of("0123456789");
      if (sz == std::string::npos)
      {
        return frameName;
      }
      return frameName.substr(0, sz);
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
    size_t singleFrameAnimIdx = 0;
    bool hasSingleFrameAnim = false;
    for (int frameNum = 0; frameNum < header->numFrames; frameNum++)
    {
      plugin_frame_pointer pluginFramePtr = framePtr[frameNum];

      if (*(pluginFramePtr.type) == SINGLE_FRAME)
      {
        // Recover pointer to the single frame
        const mdl_simpleframe_t* frame = pluginFramePtr.frames;

        std::string animationName = extract_animation_name(frame->name);
        if (!hasSingleFrameAnim || animationName != this->AnimationNames[singleFrameAnimIdx])
        {
          // New animation, emplace it
          singleFrameAnimIdx = emplace_empty_animation(animationName);
          hasSingleFrameAnim = true;
        }

        // Handle animation times
        std::vector<double>& times = this->AnimationTimes[singleFrameAnimIdx];
        if (times.empty())
        {
          times.emplace_back(0.0);
        }
        else
        {
          // Single frames are 10 fps
          times.emplace_back(times.back() + 0.1);
        }

        // Create the animation frame
        vtkSmartPointer<vtkPolyData> mesh =
          this->CreateMeshForSimpleFrame(frame, header, triangles, cells, textureCoordinates);
        this->AnimationFrames[singleFrameAnimIdx].emplace_back(mesh);
      }
      else
      {
        // Group frame are expected to be a single animation
        std::string animationName;
        std::vector<double> times;
        std::vector<vtkSmartPointer<vtkPolyData>> meshes;

        // Iterate over each frame in the group
        for (int groupFrameNum = 0; groupFrameNum < *pluginFramePtr.nb; groupFrameNum++)
        {
          // Recover the frame using the offsets because the struct does not store this pointer
          const mdl_simpleframe_t* frame = reinterpret_cast<const mdl_simpleframe_t*>(
            buffer.data() + frameOffsets[frameNum][groupFrameNum]);

          // Assume all frames are named identicaly in the group
          if (animationName.empty())
          {
            // Add a group_ prefix to identify group frames
            animationName = "group_" + extract_animation_name(frame->name);
          }

          // Recover time for this frame from the dedicated table
          times.emplace_back(pluginFramePtr.time[groupFrameNum]);

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
    // XXX: This is completely unsafe, should be rewritten using modern API
    int offset = 0;
    const mdl_header_t* header = reinterpret_cast<const mdl_header_t*>(buffer.data());
    offset += sizeof(mdl_header_t);

    // Create textures
    if (header->numSkins > 0 && header->skinWidth > 0 && header->skinHeight > 0)
    {
      this->Texture = this->CreateTexture(buffer, offset, header->skinWidth, header->skinHeight,
        static_cast<unsigned int>(header->numSkins), this->Parent->GetSkinIndex());
      if (!this->Texture)
      {
        vtkErrorWithObjectMacro(this->Parent, "Unable to read a texture, aborting.");
        return false;
      }
    }

    // Create animation frames
    bool ret = this->CreateMesh(buffer, offset, header);
    if (this->AnimationFrames.empty() || this->AnimationFrames.front().empty())
    {
      vtkErrorWithObjectMacro(
        this->Parent, "No frame read, there is nothing to display in this file.");
      return false;
    }
    return ret;
  }

  //----------------------------------------------------------------------------
  vtkF3DQuakeMDLImporter* Parent;
  std::string Description;
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  vtkSmartPointer<vtkTexture> Texture;

  std::vector<std::string> AnimationNames;
  std::vector<std::vector<double>> AnimationTimes;
  std::vector<std::vector<vtkSmartPointer<vtkPolyData>>> AnimationFrames;

  vtkIdType ActiveAnimation = -1;
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
  vtkNew<vtkActor> actor;
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(this->Internals->AnimationFrames[0][0]);
  actor->SetMapper(mapper);
  actor->GetProperty()->SetInterpolationToPBR();
  actor->GetProperty()->SetBaseColorTexture(this->Internals->Texture);
  actor->GetProperty()->SetBaseIOR(1.0);
  renderer->AddActor(actor);
  this->Internals->Mapper = mapper;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->ActorCollection->AddItem(actor);
#endif
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::UpdateAtTimeValue(double timeValue)
{
  if (this->Internals->ActiveAnimation != -1)
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
  }
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
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size()));
  assert(animationIndex >= 0);
  return this->Internals->ActiveAnimation == animationIndex;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& vtkNotUsed(nbTimeSteps), double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size()));
  assert(animationIndex >= 0);

  const std::vector<double>& times = this->Internals->AnimationTimes[animationIndex];
  // F3D does not care about timesteps, only set time range
  timeRange[0] = times.front();
  timeRange[1] = times.back();
  return true;
}
