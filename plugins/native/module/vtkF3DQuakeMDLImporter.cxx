#include "vtkF3DQuakeMDLImporter.h"
#include "vtkF3DQuakeMDLImporterConstants.h"

#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkOpenGLTexture.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkResourceStream.h>

#include <cstdint>
#include <cstring>

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

  enum FRAME_TYPE : std::uint8_t
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
  class F3DRangeError : public std::out_of_range
  {
  public:
    explicit F3DRangeError(const std::string& what = "")
      : std::out_of_range(what)
    {
    }
  };

  class F3DMathError : public std::overflow_error
  {
  public:
    explicit F3DMathError(const std::string& what = "")
      : std::overflow_error(what)
    {
    }
  };

  //----------------------------------------------------------------------------
  // Safer buffer typecasting of arbitrary buffer location
  template<typename TYPE>
  static const TYPE* PeekFromVector(const std::vector<uint8_t>& buffer, const size_t& offset)
  {
    static_assert(std::is_pod<TYPE>::value, "Vector typecast requires POD input");

    if (offset + sizeof(TYPE) > buffer.size())
    {
      throw F3DRangeError("Requested data out of range.");
    }

    return reinterpret_cast<const TYPE*>(buffer.data() + offset);
  }

  //----------------------------------------------------------------------------
  // Safer buffer typecasting with auto offset interating
  template<typename TYPE>
  static const TYPE* ReadFromVector(const std::vector<uint8_t>& buffer, size_t& offset)
  {
    const TYPE* ptr = PeekFromVector<TYPE>(buffer, offset);
    offset += sizeof(TYPE);
    return ptr;
  }

  //----------------------------------------------------------------------------
  // Safer buffer typecasting of arbitrary buffer location with variable length data
  static const mdl_simpleframe_t* PeekFromVectorSimpleframe(
    const std::vector<uint8_t>& buffer, const size_t& offset, size_t num_verts = 0)
  {
    static constexpr auto mdl_simpleframe_t_fixed_size =
      sizeof(mdl_simpleframe_t) - sizeof(mdl_simpleframe_t::verts);
    static_assert(std::is_pod<mdl_simpleframe_t>::value, "Vector typecast requires POD input");

    // check that we have enough data for the given number of verts requested
    if (offset + mdl_simpleframe_t_fixed_size + num_verts * sizeof(mdl_simpleframe_t::verts[0]) >
      buffer.size())
    {
      throw F3DRangeError("Requested data out of range.");
    }

    return reinterpret_cast<const mdl_simpleframe_t*>(buffer.data() + offset);
  }

  //----------------------------------------------------------------------------
  // Safer buffer typecasting with auto offset interating
  const mdl_simpleframe_t* ReadFromVectorSimpleframe(
    const std::vector<uint8_t>& buffer, size_t& offset, size_t num_verts = 0)
  {
    static constexpr auto mdl_simpleframe_t_fixed_size =
      sizeof(mdl_simpleframe_t) - sizeof(mdl_simpleframe_t::verts);
    auto ptr = PeekFromVectorSimpleframe(buffer, offset, num_verts);
    offset += mdl_simpleframe_t_fixed_size + num_verts * sizeof(mdl_simpleframe_t::verts[0]);
    return ptr;
  }

  //----------------------------------------------------------------------------
  explicit vtkInternals(vtkF3DQuakeMDLImporter* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkTexture> CreateTexture(const std::vector<uint8_t>& buffer, size_t& offset,
    int skinWidth, int skinHeight, unsigned int nbSkins, unsigned int skinIndex)
  {
    auto safe_mul = [](size_t a, size_t b) -> size_t
    {
      if (a != 0 && b > std::numeric_limits<size_t>::max() / a)
      {
        // overflow handling
        throw F3DMathError("Multiplication overflow");
      }
      return a * b;
    };

    auto safe_add = [](size_t a, size_t b) -> size_t
    {
      if (a != 0 && a > std::numeric_limits<size_t>::max() - b)
      {
        // overflow handling
        throw F3DMathError("Addition overflow");
      }
      return a + b;
    };

    auto make_new_skin = [&](vtkNew<vtkImageData>& skin)
    {
      // check if all the data for this operation exists
      try
      {
        size_t checkHeight = static_cast<size_t>(skinHeight) - 1;
        size_t checkWidth = static_cast<size_t>(skinWidth) - 1;
        if (skinHeight > 0 && skinWidth > 0 &&
          safe_add(offset, safe_mul(checkHeight, checkWidth)) >= buffer.size())
        {
          throw F3DRangeError("Skin dimensions out of bounds of file size");
        }
      }
      catch (const F3DMathError& e)
      {
        // Catch safe math errors and rethrow range indexing error
        throw F3DRangeError("Skin dimensions out of bounds of size_t");
      }
      skin->SetDimensions(skinWidth, skinHeight, 1);
      skin->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
      for (int x = 0; x < skinHeight; ++x)
      {
        for (int y = 0; y < skinWidth; ++y)
        {
          auto index = *vtkInternals::PeekFromVector<uint8_t>(buffer, offset + x * skinWidth + y);
          unsigned char* ptr = static_cast<unsigned char*>(skin->GetScalarPointer(y, x, 0));
          std::copy(F3DMDLDefaultColorMap[index], F3DMDLDefaultColorMap[index] + 3, ptr);
        }
      }
    };

    // Read textures.
    try
    {
      vtkNew<vtkTexture> texture;
      texture->SetColorModeToDirectScalars();
      texture->UseSRGBColorSpaceOn();

      if (skinIndex >= nbSkins)
      {
        skinIndex = 0;
        vtkWarningWithObjectMacro(
          this->Parent, "QuakeMDL.skin_index is out of bounds. Defaulting to 0.");
      }
      int skinSize = skinWidth * skinHeight;
      int groupSkinCount = 0;
      for (unsigned int i = 0; i < nbSkins; i++)
      {
        int skinGroup = *ReadFromVector<int>(buffer, offset);
        if (skinGroup == 0)
        {
          // Skip the skins that are not selected
          if (i == skinIndex)
          {
            vtkNew<vtkImageData> img;
            make_new_skin(img);
            texture->SetInputData(img);
          }
          offset += skinSize * sizeof(int8_t);
        }
        else
        {
          std::string skinAnimationName = "skin_" + std::to_string(groupSkinCount);
          this->GroupSkinAnimationNames.emplace_back(skinAnimationName);
          auto nb = *ReadFromVector<int>(buffer, offset);
          this->GroupSkins.emplace_back(nb);
          this->GroupSkinDurations.emplace_back(nb + 1, 0.0f);
          for (int j = 1; j <= nb; ++j)
          {
            auto timeValue = *ReadFromVector<float>(buffer, offset);
            this->GroupSkinDurations[groupSkinCount][j] = static_cast<double>(timeValue);
          }
          for (int skinIdx = 0; skinIdx < nb; ++skinIdx)
          {
            vtkNew<vtkImageData> skinTemp;
            make_new_skin(skinTemp);
            this->GroupSkins[groupSkinCount][skinIdx] = skinTemp;
            offset += skinSize * sizeof(int8_t);
          }
          if (i == skinIndex)
          {
            texture->SetInputData(this->GroupSkins[groupSkinCount].front());
          }
          ++groupSkinCount;
        }
      }
      return texture;
    }
    catch (const F3DRangeError& e)
    {
      // Catch fatal errors thrown from overrunning the buffer
      vtkErrorWithObjectMacro(this->Parent, "CreateTexture Accessed data out of range, aborting.");
      return nullptr;
    }
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
  bool CreateMesh(const std::vector<uint8_t>& buffer, size_t offset, const mdl_header_t* header)
  {
    try
    {
      const unsigned long totalProgress = header->numFrames * 2 + header->numTriangles;
      unsigned long currentProgress = 0;
      double progressRate;

      constexpr int mdl_simpleframe_t_fixed_size = sizeof(mdl_simpleframe_t) -
        sizeof(mdl_simpleframe_t::verts); // Size of bboxmin, bboxmax and name.

      // Read Texture Coordinates
      auto texcoords = PeekFromVector<mdl_texcoord_t>(buffer, offset);
      offset += sizeof(mdl_texcoord_t) * header->numVertices;

      // Read Triangles
      auto triangles = PeekFromVector<mdl_triangle_t>(buffer, offset);
      offset += sizeof(mdl_triangle_t) * header->numTriangles;

      // Read frames
      std::vector<plugin_frame_pointer> framePtr =
        std::vector<plugin_frame_pointer>(header->numFrames);
      std::vector<std::vector<int>> frameOffsets = std::vector<std::vector<int>>();
      for (int i = 0; i < header->numFrames; i++)
      {
        framePtr[i].type = PeekFromVector<int>(buffer, offset);
        if (*framePtr[i].type == SINGLE_FRAME)
        {
          // Alias offset
          auto offsetAlias = offset + sizeof(int32_t);
          framePtr[i].nb = nullptr;
          framePtr[i].time = nullptr;

          // Size of a frame is mdl_simpleframe_t_fixed_size + mdl_vertex_t * numVertices, +
          // sizeof(int)

          // Note : mdl_simpleframe_t can have *up to and including* 1024 verts. So if this data is
          // the last in the file the peek_at_vector func will error out. As such we use a different
          // helper.
          framePtr[i].frames = ReadFromVectorSimpleframe(buffer, offsetAlias, header->numVertices);
          // Apply alias
          offset = offsetAlias;

          // Always emplace in case of mixed single frame and group frame
          frameOffsets.emplace_back(std::vector<int>());
        }
        else
        {
          // Alias offset
          auto offsetAlias = offset + sizeof(int32_t);
          framePtr[i].nb = ReadFromVector<int>(buffer, offsetAlias);
          // Skips parameters min and max.
          offsetAlias += (2 * sizeof(mdl_vertex_t));
          framePtr[i].time = PeekFromVector<float>(buffer, offsetAlias);
          // Points to the first frame, 4 * nbFrames for the float array
          // note : see above
          framePtr[i].frames = PeekFromVectorSimpleframe(
            buffer, offsetAlias + (*framePtr[i].nb) * sizeof(float), header->numVertices);

          offsetAlias += (*framePtr[i].nb) * sizeof(float);
          // Apply alias
          offset = offsetAlias;

          frameOffsets.emplace_back(std::vector<int>());

          // check that we won't run off the buffer during loop
          if (offset +
              (*framePtr[i].nb *
                (mdl_simpleframe_t_fixed_size + sizeof(mdl_vertex_t) * header->numVertices)) >
            buffer.size())
          {
            throw F3DRangeError("Requested data out of range.");
          }

          for (int j = 0; j < *framePtr[i].nb; j++)
          {
            // Offset for each frame
            frameOffsets[i].emplace_back(offset);
            offset += mdl_simpleframe_t_fixed_size + sizeof(mdl_vertex_t) * header->numVertices;
          }
        }

        currentProgress++;
        if (i % 128 == 0)
        {
          progressRate = static_cast<double>(currentProgress) / totalProgress;
          this->Parent->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void*>(&progressRate));
        }
      }

      // Draw cells and scale texture coordinates
      vtkNew<vtkCellArray> cells;
      cells->AllocateExact(header->numTriangles, 3 * header->numTriangles);
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

        currentProgress++;
        if (i % 128 == 0)
        {
          progressRate = static_cast<double>(currentProgress) / totalProgress;
          this->Parent->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void*>(&progressRate));
        }
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
          // Single frames are 10 fps
          times.emplace_back(times.back() + 0.1);

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

          // groupFrames always start at 0.0
          times.emplace_back(0.0);
          // Iterate over each frame in the group
          for (int groupFrameNum = 0; groupFrameNum < *pluginFramePtr.nb; groupFrameNum++)
          {

            // Recover the frame using the offsets because the struct does not store this pointer
            auto frame = PeekFromVectorSimpleframe(
              buffer, frameOffsets[frameNum][groupFrameNum], header->numVertices);

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

        currentProgress++;
        if (frameNum % 128 == 0)
        {
          progressRate = static_cast<double>(currentProgress) / totalProgress;
          this->Parent->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void*>(&progressRate));
        }
      }

      currentProgress = totalProgress;
      progressRate = 1.0;
      this->Parent->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void*>(&progressRate));
      return true;
    }
    catch (const F3DRangeError& e)
    {
      // Catch fatal errors thrown from overrunning the buffer
      vtkErrorWithObjectMacro(this->Parent, "CreateMesh Accessed data out of range, aborting.");
      return false;
    }
  }

  //----------------------------------------------------------------------------
  bool ReadScene(vtkResourceStream* stream)
  {
    // Recover length of stream
    stream->Seek(0, vtkResourceStream::SeekDirection::End);
    size_t length = stream->Tell();
    stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

    // Read stream into buffer
    std::vector<uint8_t> buffer(length);
    stream->Read(buffer.data(), length);

    // Read header
    // XXX: This is completely unsafe, should be rewritten using modern API
    size_t offset = 0;
    const mdl_header_t* header;
    try
    {
      header = ReadFromVector<mdl_header_t>(buffer, offset);
    }
    catch (const F3DRangeError& e)
    {
      vtkErrorWithObjectMacro(this->Parent, "Unable to read header, aborting.");
      return false;
    }

    // check magic number for "IPDO" or "IDST"
    if (!(header->IDPO == 1330660425 || header->IDPO == 1414743113))
    {
      vtkErrorWithObjectMacro(this->Parent, "Header Magic number incorrect, aborting.");
      return false;
    }

    // check version number for numbers under 10
    if (header->version > 10)
    {
      // TODO : Should probably check for exact version numbers. This info isn't easy to come by.
      vtkErrorWithObjectMacro(this->Parent, "Unsupported MDL version");
      return false;
    }

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
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  vtkSmartPointer<vtkTexture> Texture;

  std::vector<std::string> AnimationNames;
  std::vector<std::vector<double>> AnimationTimes;
  std::vector<std::vector<vtkSmartPointer<vtkPolyData>>> AnimationFrames;

  std::vector<std::string> GroupSkinAnimationNames;
  std::vector<std::vector<vtkSmartPointer<vtkImageData>>> GroupSkins;
  std::vector<std::vector<double>> GroupSkinDurations;

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
  // Stream is higher priority than filename.
  vtkResourceStream* stream = this->GetStream();
  vtkNew<vtkFileResourceStream> fileStream;
  if (!stream)
  {
    if (!fileStream->Open(this->GetFileName()))
    {
      vtkErrorMacro("Unable to open " << this->GetFileName() << " , aborting.");
      return 0;
    }

    stream = fileStream;
  }

  return this->Internals->ReadScene(stream);
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
    // Find frameIndex for the provided timeValue so that t0 <= timeValue < t1
    // Animation range is from [Start time of first frame, Finish time of last frame]

    // First time >= value, excluding the last element as it only represents finish time of the last
    // frame

    // Animation in AnimationFrames are mesh animations and at greater indices texture animations
    // are rendered
    bool isMeshAnimation = this->Internals->ActiveAnimation <
      static_cast<vtkIdType>(this->Internals->AnimationNames.size());
    size_t animIndex = isMeshAnimation
      ? this->Internals->ActiveAnimation
      : this->Internals->ActiveAnimation - this->Internals->AnimationNames.size();
    const std::vector<double>& times = isMeshAnimation
      ? this->Internals->AnimationTimes[animIndex]
      : this->Internals->GroupSkinDurations[animIndex];
    const auto found = std::lower_bound(times.begin(), times.end() - 1, timeValue);
    // If found at finish time of last frame, select last frame's start time (second last value),
    // else select distance
    const size_t i =
      (found == times.end() - 1) ? times.size() - 2 : std::distance(times.begin(), found);
    // If time at index i > timeValue, then choose the previous frame
    const size_t frameIndex = times[i] > timeValue && i > 0 ? i - 1 : i;
    if (isMeshAnimation)
    {
      this->Internals->Mapper->SetInputData(
        this->Internals->AnimationFrames[animIndex][frameIndex]);
    }
    else
    {
      this->Internals->Texture->SetInputData(this->Internals->GroupSkins[animIndex][frameIndex]);
    }
  }
  return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DQuakeMDLImporter::GetNumberOfAnimations()
{
  return static_cast<vtkIdType>(
    this->Internals->AnimationNames.size() + this->Internals->GroupSkinAnimationNames.size());
}

//----------------------------------------------------------------------------
std::string vtkF3DQuakeMDLImporter::GetAnimationName(vtkIdType animationIndex)
{
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size() +
                            this->Internals->GroupSkinAnimationNames.size()));
  assert(animationIndex >= 0);

  if (animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size()))
  {
    return this->Internals->AnimationNames[animationIndex];
  }
  // Skin group animations are rendered for indices larger than frame animations
  return this->Internals
    ->GroupSkinAnimationNames[animationIndex - this->Internals->AnimationNames.size()];
}

//----------------------------------------------------------------------------
void vtkF3DQuakeMDLImporter::EnableAnimation(vtkIdType animationIndex)
{
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size() +
                            this->Internals->GroupSkinAnimationNames.size()));
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
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size() +
                            this->Internals->GroupSkinAnimationNames.size()));
  assert(animationIndex >= 0);
  return this->Internals->ActiveAnimation == animationIndex;
}

//----------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& nbTimeSteps, double timeRange[2], vtkDoubleArray* timeSteps)
{
  assert(animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size() +
                            this->Internals->GroupSkinAnimationNames.size()));
  assert(animationIndex >= 0);

  const std::vector<double>& times =
    animationIndex < static_cast<vtkIdType>(this->Internals->AnimationNames.size())
    ? this->Internals->AnimationTimes[animationIndex]
    : this->Internals->GroupSkinDurations[animationIndex - this->Internals->AnimationNames.size()];

  timeRange[0] = times.front();
  // If single frame, keep animation duration = 0
  timeRange[1] = times.size() == 2 ? times.front() : times.back();

  nbTimeSteps = static_cast<int>(times.size());
  timeSteps->SetNumberOfTuples(times.size());

  for (unsigned int i = 0; i < times.size(); ++i)
  {
    timeSteps->SetValue(i, times[i]);
  }
  return true;
}
