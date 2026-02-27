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

#include <vtkInterpolateDataSetAttributes.h>
#include <vtkMathUtilities.h>

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
    int ident;
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
  // Safer buffer typecasting with auto offset incrementing
  template<typename TYPE>
  static const TYPE* ReadFromVector(const std::vector<uint8_t>& buffer, size_t& offset)
  {
    const TYPE* ptr = vtkInternals::PeekFromVector<TYPE>(buffer, offset);
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
  static bool ReadAndCheckHeader(const std::vector<uint8_t>& buffer, vtkObject* object,
    size_t& offset, const mdl_header_t*& header)
  {
    // Read header
    try
    {
      header = vtkInternals::ReadFromVector<mdl_header_t>(buffer, offset);
    }
    catch (const F3DRangeError&)
    {
      if (object)
      {
        vtkErrorWithObjectMacro(object, "Invalid MDL file");
      }
      return false;
    }

    // Check magic number for "IPDO" or "IDST"
    if (!(header->ident == 0x4F504449 || header->ident == 0x54534449))
    {
      if (object)
      {
        vtkErrorWithObjectMacro(object, "Incompatible MDL header");
      }
      return false;
    }

    // Check version for v6 exactly
    if (header->version != 6)
    {
      if (object)
      {
        vtkErrorWithObjectMacro(object, "Unsupported MDL version. Only version 6 is supported");
      }
      return false;
    }
    return true;
  }

  //----------------------------------------------------------------------------
  // Safer buffer typecasting with auto offset incrementing with variable length data
  static const mdl_simpleframe_t* ReadFromVectorSimpleframe(
    const std::vector<uint8_t>& buffer, size_t& offset, size_t num_verts = 0)
  {
    static constexpr auto mdl_simpleframe_t_fixed_size =
      sizeof(mdl_simpleframe_t) - sizeof(mdl_simpleframe_t::verts);
    auto ptr = vtkInternals::PeekFromVectorSimpleframe(buffer, offset, num_verts);
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
    auto make_new_skin = [&](vtkNew<vtkImageData>& skin)
    {
      // check if all the data for this operation exists
      uint64_t checkHeight = static_cast<uint64_t>(skinHeight) - 1;
      uint64_t checkWidth = static_cast<uint64_t>(skinWidth) - 1;
      uint64_t checkSize = static_cast<uint64_t>(offset) + (checkHeight * checkWidth);
      if (skinHeight > 0 && skinWidth > 0 && (checkSize >= static_cast<uint64_t>(buffer.size())))
      {
        throw F3DRangeError("Skin dimensions out of bounds of file size");
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
        int skinGroup = *vtkInternals::ReadFromVector<int>(buffer, offset);
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
          auto nb = *vtkInternals::ReadFromVector<int>(buffer, offset);
          this->GroupSkins.emplace_back(nb);
          this->GroupSkinDurations.emplace_back(nb + 1, 0.0f);
          for (int j = 1; j <= nb; ++j)
          {
            auto timeValue = *vtkInternals::ReadFromVector<float>(buffer, offset);
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
    catch (const F3DRangeError&)
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
      auto texcoords = vtkInternals::PeekFromVector<mdl_texcoord_t>(buffer, offset);
      offset += sizeof(mdl_texcoord_t) * header->numVertices;

      // Read Triangles
      auto triangles = vtkInternals::PeekFromVector<mdl_triangle_t>(buffer, offset);
      offset += sizeof(mdl_triangle_t) * header->numTriangles;

      // Read frames
      std::vector<plugin_frame_pointer> framePtr =
        std::vector<plugin_frame_pointer>(header->numFrames);
      std::vector<std::vector<size_t>> frameOffsets = std::vector<std::vector<size_t>>();
      for (int i = 0; i < header->numFrames; i++)
      {
        framePtr[i].type = vtkInternals::PeekFromVector<int>(buffer, offset);
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
          framePtr[i].frames =
            vtkInternals::ReadFromVectorSimpleframe(buffer, offsetAlias, header->numVertices);
          // Apply alias
          offset = offsetAlias;

          // Always emplace in case of mixed single frame and group frame
          frameOffsets.emplace_back(std::vector<size_t>());
        }
        else
        {
          // Alias offset
          auto offsetAlias = offset + sizeof(int32_t);
          framePtr[i].nb = vtkInternals::ReadFromVector<int>(buffer, offsetAlias);
          // Skips parameters min and max.
          offsetAlias += (2 * sizeof(mdl_vertex_t));
          framePtr[i].time = vtkInternals::PeekFromVector<float>(buffer, offsetAlias);
          // Points to the first frame, 4 * nbFrames for the float array
          // note : see above
          framePtr[i].frames = vtkInternals::PeekFromVectorSimpleframe(
            buffer, offsetAlias + (*framePtr[i].nb) * sizeof(float), header->numVertices);

          offsetAlias += (*framePtr[i].nb) * sizeof(float);
          // Apply alias
          offset = offsetAlias;

          frameOffsets.emplace_back(std::vector<size_t>());

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
            auto frame = vtkInternals::PeekFromVectorSimpleframe(
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
    catch (const F3DRangeError&)
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
    // TODO rework to avoid copying the whole buffer
    std::vector<uint8_t> buffer(length);
    stream->Read(buffer.data(), length);

    // Read Header
    size_t offset = 0;
    const mdl_header_t* header;
    if (!vtkInternals::ReadAndCheckHeader(buffer, this->Parent, offset, header))
    {
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

  vtkSmartPointer<vtkInterpolateDataSetAttributes> Interpolator;
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

    // Need at least 2 time values
    if (times.size() < 2)
    {
      return true;
    }
    // Clamping timeValue to the valid animation range
    double clampedTime = std::max(times.front(), std::min(timeValue, times.back()));
    // Find the frame interval containing our time
    auto upperIt = std::lower_bound(times.begin(), times.end() - 1, clampedTime);
    size_t upperIndex = std::distance(times.begin(), upperIt);

    // Get the two frame indices we're interploating between
    size_t frameIndex0, frameIndex1;
    double t0, t1;

    if (upperIndex == 0)
    {
      // Before or at the first frame
      frameIndex0 = 0;
      frameIndex1 = 0;
      t0 = times[0];
      t1 = times[0];
    }
    else if (upperIndex >= times.size() - 1)
    {
      // At or past the last frame
      frameIndex0 = times.size() - 2;
      frameIndex1 = times.size() - 2;
      t0 = times[times.size() - 2];
      t1 = times[times.size() - 2];
    }
    else
    {
      // between two frames
      if (times[upperIndex] > clampedTime)
      {
        // Time falls between [upperIndex-1] and [upperIndex]
        frameIndex0 = upperIndex - 1;
        frameIndex1 = upperIndex;
      }
      else
      {
        // Time exactly at upperIndex or between [upperIndex] and [upperIndex+1]
        frameIndex0 = upperIndex;
        frameIndex1 = std::min(upperIndex + 1, times.size() - 2);
      }

      t0 = times[frameIndex0];
      t1 = times[frameIndex1];
    }

    // Calculate interpolation factor (alpha)
    double alpha = 0.0;
    if (frameIndex0 != frameIndex1)
    {
      alpha = vtkMathUtilities::SafeDivision(clampedTime - t0, t1 - t0);
      alpha = std::max(0.0, std::min(1.0, alpha));
    }

    if (isMeshAnimation)
    {
      if (frameIndex0 == frameIndex1)
      {
        // No interpolation needed: use frame0
        this->Internals->Mapper->SetInputData(
          this->Internals->AnimationFrames[animIndex][frameIndex0]);
      }
      else
      {
        // Interpolate between frame0 and frame1
        // using <vtkInterpolateDataSetAttributes>
        if (!this->Internals->Interpolator)
        {
          this->Internals->Interpolator = vtkSmartPointer<vtkInterpolateDataSetAttributes>::New();
        }

        // Clear previous inputs and add new ones
        this->Internals->Interpolator->RemoveAllInputs();
        this->Internals->Interpolator->AddInputData(
          this->Internals->AnimationFrames[animIndex][frameIndex0]);
        this->Internals->Interpolator->AddInputData(
          this->Internals->AnimationFrames[animIndex][frameIndex1]);
        this->Internals->Interpolator->SetT(alpha);
        this->Internals->Interpolator->Update();

        // Get the interpolated mesh and set it to the mapper
        vtkPolyData* interpolatedMesh = this->Internals->Interpolator->GetPolyDataOutput();
        this->Internals->Mapper->SetInputData(interpolatedMesh);
      }
    }
    else
    {
      // For texture animations, use nearest neighbor approach
      // Not interpolating here since texture interpolation is complex(?)
      size_t textureIndex = alpha < 0.5 ? frameIndex0 : frameIndex1;
      this->Internals->Texture->SetInputData(this->Internals->GroupSkins[animIndex][textureIndex]);
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
bool vtkF3DQuakeMDLImporter::GetTemporalInformation(
  vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps, vtkDoubleArray* timeSteps)
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

//------------------------------------------------------------------------------
bool vtkF3DQuakeMDLImporter::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }

  // Read header into buffer
  std::size_t headerSize = sizeof(vtkInternals::mdl_header_t);
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  std::vector<uint8_t> buffer(headerSize);
  if (stream->Read(buffer.data(), headerSize) != headerSize)
  {
    return false;
  }

  // Check header buffer
  size_t offset = 0;
  const vtkInternals::mdl_header_t* header;
  if (!vtkInternals::ReadAndCheckHeader(buffer, nullptr, offset, header))
  {
    return false;
  }
  return true;
}
