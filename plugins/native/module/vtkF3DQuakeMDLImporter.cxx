#include "vtkF3DQuakeMDLImporter.h"
#include "vtkF3DQuakeMDLImporterConstants.h"

#include <cstddef>
#include <vector>
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
  template<typename TYPE>
  static TYPE ReadFromStream(vtkResourceStream* stream)
  {
    TYPE ret;
    if (stream->Read(&ret, sizeof(ret)) != sizeof(ret))
    {
      throw F3DRangeError("Requested data out of range.");
    }
    return ret;
  }

  //----------------------------------------------------------------------------
  template<typename TYPE>
  std::vector<TYPE> ReadVectorFromStream(vtkResourceStream* stream, size_t count)
  {
    const size_t remainingBytes = this->streamSize - stream->Tell();
    if (count > remainingBytes / sizeof(TYPE))
    {
      throw F3DRangeError("Requested data out of range.");
    }
    std::vector<TYPE> vec(count);
    stream->Read(vec.data(), vec.size() * sizeof(TYPE));
    return vec;
  }

  //----------------------------------------------------------------------------
  static bool ReadAndCheckHeader(vtkResourceStream* stream, vtkObject* object, mdl_header_t& header)
  {
    // Read header
    try
    {
      header = vtkInternals::ReadFromStream<mdl_header_t>(stream);
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
    if (!(header.ident == 0x4F504449 || header.ident == 0x54534449))
    {
      if (object)
      {
        vtkErrorWithObjectMacro(object, "Incompatible MDL header");
      }
      return false;
    }

    // Check version for v6 exactly
    if (header.version != 6)
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
  explicit vtkInternals(vtkF3DQuakeMDLImporter* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkTexture> CreateTexture(vtkResourceStream* stream, int skinWidth,
    int skinHeight, unsigned int nbSkins, unsigned int skinIndex)
  {
    auto make_new_skin = [&](vtkNew<vtkImageData>& skin)
    {
      const size_t skinByteCount = static_cast<size_t>(skinWidth) * skinHeight;

      std::vector<uint8_t> skinData = this->ReadVectorFromStream<uint8_t>(stream, skinByteCount);

      skin->SetDimensions(skinWidth, skinHeight, 1);
      skin->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
      for (int x = 0; x < skinHeight; ++x)
      {
        for (int y = 0; y < skinWidth; ++y)
        {
          const auto index = skinData[x * skinWidth + y];
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
      const size_t skinSize = static_cast<size_t>(skinWidth) * skinHeight;
      int groupSkinCount = 0;
      for (unsigned int i = 0; i < nbSkins; i++)
      {
        const int skinGroup = vtkInternals::ReadFromStream<int>(stream);
        if (skinGroup == 0)
        {
          // Skip the skins that are not selected
          if (i == skinIndex)
          {
            vtkNew<vtkImageData> img;
            make_new_skin(img);
            texture->SetInputData(img);
          }
          else
          {
            // move stream cursor to skip unselected skin
            stream->Seek(skinSize * sizeof(int8_t), vtkResourceStream::SeekDirection::Current);
          }
        }
        else
        {
          const std::string skinAnimationName = "skin_" + std::to_string(groupSkinCount);
          this->GroupSkinAnimationNames.emplace_back(skinAnimationName);
          auto nb = vtkInternals::ReadFromStream<int>(stream);
          this->GroupSkins.emplace_back(nb);
          this->GroupSkinDurations.emplace_back(nb + 1, 0.0f);
          for (int j = 1; j <= nb; ++j)
          {
            auto timeValue = vtkInternals::ReadFromStream<float>(stream);
            this->GroupSkinDurations[groupSkinCount][j] = static_cast<double>(timeValue);
          }
          for (int skinIdx = 0; skinIdx < nb; ++skinIdx)
          {
            vtkNew<vtkImageData> skinTemp;
            make_new_skin(skinTemp);
            this->GroupSkins[groupSkinCount][skinIdx] = skinTemp;
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

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260320)
    vertices->Reserve(header->numTriangles * 3);
#else
    vertices->Allocate(header->numTriangles * 3);
#endif

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
        const int normalIndex = frame->verts[vertexNum[j]].normalIndex;
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
  bool CreateMesh(vtkResourceStream* stream, const mdl_header_t& header)
  {
    try
    {
      if (header.numVertices < 0 || header.numVertices > 1024)
      {
        throw F3DRangeError("Invalid number of vertices");
      }

      const unsigned long totalProgress = header.numFrames + header.numTriangles;
      unsigned long currentProgress = 0;
      double progressRate;
      constexpr int mdl_simpleframe_t_fixed_size = sizeof(mdl_simpleframe_t) -
        sizeof(mdl_simpleframe_t::verts); // Size of bboxmin, bboxmax and name.

      // Read Texture Coordinates
      std::vector<mdl_texcoord_t> texcoords =
        this->ReadVectorFromStream<mdl_texcoord_t>(stream, header.numVertices);

      // Read Triangles
      std::vector<mdl_triangle_t> triangles =
        this->ReadVectorFromStream<mdl_triangle_t>(stream, header.numTriangles);

      // Draw cells and scale texture coordinates
      vtkNew<vtkCellArray> cells;
      cells->AllocateExact(header.numTriangles, 3 * header.numTriangles);
      vtkNew<vtkFloatArray> textureCoordinates;
      textureCoordinates->SetNumberOfComponents(2);
      textureCoordinates->SetName("TextureCoordinates");

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260320)
      textureCoordinates->ReserveValues(header.numTriangles * 3);
#else
      textureCoordinates->Allocate(header.numTriangles * 3);
#endif
      for (int i = 0; i < header.numTriangles; i++)
      {
        for (const int vertex : triangles[i].vertex)
        {
          float coord_s = texcoords[vertex].coord_s;
          float coord_t = texcoords[vertex].coord_t;
          if (!triangles[i].facesFront && texcoords[vertex].onseam)
          {
            coord_s = coord_s + header.skinWidth * 0.5f; // Backface
          }

          // Scale s and t to range from 0.0 to 1.0
          coord_s = (coord_s + 0.5) / header.skinWidth;
          coord_t = (coord_t + 0.5) / header.skinHeight;
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
      const size_t frameSize =
        mdl_simpleframe_t_fixed_size + sizeof(mdl_vertex_t) * header.numVertices;
      for (int frameNum = 0; frameNum < header.numFrames; frameNum++)
      {
        const int frameType = vtkInternals::ReadFromStream<int>(stream);
        if (frameType == SINGLE_FRAME)
        {
          // Recover frame using stream
          mdl_simpleframe_t frame;
          if (stream->Read(&frame, frameSize) != frameSize)
          {
            throw F3DRangeError("Invalid frame size");
          }

          const std::string animationName = extract_animation_name(frame.name);
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
          const vtkSmartPointer<vtkPolyData> mesh = this->CreateMeshForSimpleFrame(
            &frame, &header, triangles.data(), cells, textureCoordinates);
          this->AnimationFrames[singleFrameAnimIdx].emplace_back(mesh);
        }
        else
        {
          // Group frame are expected to be a single animation
          std::string animationName;
          std::vector<double> times;
          std::vector<vtkSmartPointer<vtkPolyData>> meshes;

          const size_t nb = vtkInternals::ReadFromStream<int>(stream);

          // skip bounding box
          stream->Seek(2 * sizeof(mdl_vertex_t), vtkResourceStream::SeekDirection::Current);

          std::vector<float> frameTimes = this->ReadVectorFromStream<float>(stream, nb);

          // groupFrames always start at 0.0
          times.emplace_back(0.0);
          // Iterate over each frame in the group
          for (size_t groupFrameNum = 0; groupFrameNum < nb; groupFrameNum++)
          {

            // Recover the frame using stream
            mdl_simpleframe_t frame;
            if (stream->Read(&frame, frameSize) != frameSize)
            {
              throw F3DRangeError("Invalid frame size");
            }

            // Assume all frames are named identicaly in the group
            if (animationName.empty())
            {
              // Add a group_ prefix to identify group frames
              animationName = "group_" + extract_animation_name(frame.name);
            }

            // Recover time for this frame from the dedicated table
            times.emplace_back(frameTimes[groupFrameNum]);

            // Recover mesh for this frame
            meshes.emplace_back(this->CreateMeshForSimpleFrame(
              &frame, &header, triangles.data(), cells, textureCoordinates));
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
    this->streamSize = stream->Tell();
    stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

    // Read Header
    mdl_header_t header;
    if (!vtkInternals::ReadAndCheckHeader(stream, this->Parent, header))
    {
      return false;
    }

    // Create textures
    if (header.numSkins > 0 && header.skinWidth > 0 && header.skinHeight > 0)
    {
      this->Texture = this->CreateTexture(stream, header.skinWidth, header.skinHeight,
        static_cast<unsigned int>(header.numSkins), this->Parent->GetSkinIndex());
      if (!this->Texture)
      {
        vtkErrorWithObjectMacro(this->Parent, "Unable to read a texture, aborting.");
        return false;
      }
    }

    // Create animation frames
    const bool ret = this->CreateMesh(stream, header);
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
  size_t streamSize = 0;
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
  this->ActorCollection->AddItem(actor);
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
    const bool isMeshAnimation = this->Internals->ActiveAnimation <
      static_cast<vtkIdType>(this->Internals->AnimationNames.size());
    const size_t animIndex = isMeshAnimation
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

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  // Check header buffer
  vtkInternals::mdl_header_t header;
  return vtkInternals::ReadAndCheckHeader(stream, nullptr, header);
}
