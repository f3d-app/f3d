#include "vtkQuakeMDLImporter.h"

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
vtkStandardNewMacro(vtkQuakeMDLImporter);

class vtkQuakeMDLImporter::vtkInternals
{
public:
  //----------------------------------------------------------------------------
  explicit vtkInternals(vtkQuakeMDLImporter* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  void ImportCameras(vtkRenderer* renderer)
  {
    double cameraPosition[3] = { 50.0, 15.0, 0.0 };
    vtkNew<vtkTransform> roll;
    roll->RotateX(270);
    roll->RotateZ(270);
    renderer->GetActiveCamera()->SetPosition(cameraPosition);
    renderer->GetActiveCamera()->SetModelTransformMatrix(roll->GetMatrix());
  }

  //----------------------------------------------------------------------------
  void ImportLights(vtkRenderer* renderer)
  {
    // Adds 3 lights, there's already one on the right side.

    vtkNew<vtkLight> frontLight;
    double frontLightPosition[3] = { 50.0, 50.0, 0.0 };
    frontLight->SetPosition(frontLightPosition);
    vtkNew<vtkLight> leftLight;
    double leftLightPosition[3] = { 50.0, -50.0, 0.0 };
    leftLight->SetPosition(leftLightPosition);
    vtkNew<vtkLight> backLight;
    double backLightPosition[3] = { -50.0, 0.0, 0.0 };
    backLight->SetPosition(backLightPosition);

    renderer->AddLight(frontLight);
    renderer->AddLight(leftLight);
    renderer->AddLight(backLight);
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkTexture> CreateTexture(std::vector<unsigned char> buffer, int& offset,
    int skinWidth, int skinHeight, int nbSkins, int selectedSkinIndex)
  {
    vtkNew<vtkTexture> texture;
    texture->InterpolateOn();

    // Read textures.
    struct mdl_skin_t
    {

      //int group;           /* 0 = single, 1 = group */
      //unsigned char* data; /* texture data */
    };
    struct mdl_groupskin_t
    {
      //int group;            /* 1 = group */
      //int nb;               /* number of pics */
      //float* time;          /* time duration for each pic */
      //unsigned char** data; /* texture data */
    };
    struct mixed_pointer_array
    {
      int group;
      unsigned char* skin;
    };
    mixed_pointer_array* skins = new mixed_pointer_array[nbSkins];
    // unsigned char* skin = buffer.data() + offset + 4;
    for (int i = 0; i < nbSkins; i++)
    {
      int* group = reinterpret_cast<int*>(buffer.data() + offset);
      if (*group == 0)
      {
        skins[i].group = 0;
        skins[i].skin = reinterpret_cast<unsigned char*>(buffer.data() + 4 + offset);
        offset += 4 + (skinWidth) * (skinHeight);
      }
      else
      {
        skins[i].group = 1;
        int nb = *reinterpret_cast<int*>(buffer.data() + offset + 4);
        skins[i].skin = reinterpret_cast<unsigned char*>(buffer.data() + 4 + nb * 4 + offset);
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
        unsigned char index = *reinterpret_cast<unsigned char*>(selectedSkin + i * skinWidth + j);
        unsigned char* ptr = reinterpret_cast<unsigned char*>(img->GetScalarPointer(j, i, 0));
        ptr[0] = DefaultColorMap[index][0]; // R
        ptr[1] = DefaultColorMap[index][1]; // G
        ptr[2] = DefaultColorMap[index][2]; // B
      }
    }

    texture->SetInputData(img);
    return texture;
  }

  void CreateMesh(std::vector<unsigned char> buffer, int offset, mdl_header_t* header)
  {
    // Read texture coordinates
    struct mdl_texcoord_t
    {
      int onseam;
      int s;
      int t;
    };
    mdl_texcoord_t* texcoords = reinterpret_cast<mdl_texcoord_t*>(buffer.data() + offset);
    offset += 12 * header->numVertices;
    // Read triangles
    struct mdl_triangle_t
    {
      int facesfront; /* 0 = backface, 1 = frontface */
      int vertex[3];  /* vertex indices */
    };
    mdl_triangle_t* triangles = reinterpret_cast<mdl_triangle_t*>(buffer.data() + offset);
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
    struct mdl_frame_t
    {
//      int type;
      mdl_simpleframe_t frame;
    };
    struct mdl_groupframe_t
    {
//      int type;
//      int nb;
      mdl_vertex_t min;
      mdl_vertex_t max;
//      float* time;               // Size is nbFrames ???
//      mdl_simpleframe_t* frames; // Size is nbFrames ???
    };
    struct plugin_frame_pointer
    {
      int* type;
      int* nb;
      float* time;
      mdl_simpleframe_t* frames;
    };
    plugin_frame_pointer* framePtr = new plugin_frame_pointer[header->numFrames];
    for (int i = 0; i < header->numFrames; i++)
    {
      int* type = reinterpret_cast<int*>(buffer.data() + offset);
      if (*type == 0)
      {
        framePtr[i].type = type;
        framePtr[i].nb = nullptr;
        framePtr[i].time = nullptr;
        framePtr[i].frames = reinterpret_cast<mdl_simpleframe_t*>(buffer.data() + 4 + offset);
        offset += 4 + 24 + 4 * (header->numVertices);
      }
      else
      {
        framePtr[i].type = type;
        framePtr[i].nb = reinterpret_cast<int*>(buffer.data() + 4 + offset);
        // mdl_vertex_t* min = reinterpret_cast<mdl_vertex_t*>(buffer.data() + 8 + offset);
        // mdl_vertex_t* max = reinterpret_cast<mdl_vertex_t*>(buffer.data() + 12 + offset);
        // float* time = framePtr[i].time = reinterpret_cast<float*>(buffer.data() + 16 + offset);
        framePtr[i].frames =
          reinterpret_cast<mdl_simpleframe_t*>(buffer.data() + 16 + 4 * (*framePtr[i].nb) + offset);
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
    plugin_texture_coords* coords = new plugin_texture_coords[header->numTriangles * 3];
    for (int i = 0; i < header->numTriangles; i++)
    {
      vtkIdType* vertexNum = new vtkIdType[3];
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
          vtkIdType* vertexNum = new vtkIdType[3];
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
            normals->SetTuple3(i * 3 + j, NormalVectors[normalIndex][0] / 255.0,
              NormalVectors[normalIndex][1] / 255.0, NormalVectors[normalIndex][2] / 255.0);
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
        }
        else if (meshName != frameName)
        {
          frameIndex++;
          frameName = meshName;
          NumberOfAnimations++;
        }
        std::pair<int, float> pair = std::make_pair(frameIndex, 0.0);
        GroupAndTimeVal.emplace_back(pair);
      }
      else
      {
        for (int groupFrameNum = 0; groupFrameNum < *selectedFrame.nb; groupFrameNum++)
        {
          for (int i = 0; i < header->numTriangles; i++)
          {
            vtkIdType* vertexNum = new vtkIdType[3];
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
              normals->SetTuple3(i * 3 + j, NormalVectors[normalIndex][0] / 255.0,
                NormalVectors[normalIndex][1] / 255.0, NormalVectors[normalIndex][2] / 255.0);
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
          }
          else if (meshName != frameName)
          {
            frameIndex++;
            NumberOfAnimations++;
            frameName = meshName;
          }
          std::pair<int, float> pair =
            std::make_pair(frameIndex, selectedFrame.time[groupFrameNum]);
          GroupAndTimeVal.emplace_back(pair);
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
          double* interp = new double[3];
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

  void UpdateFrame(double timeValue)
  {
    // Hardcoded frames per second, 60FPS
    if (abs(timeValue - LastRenderTime) > 1.0 / 60)
    {
      Mapper->SetInputData(
        Mesh[(CurrentFrameIndex++) % (LastFrameIndex - FirstFrameIndex) + FirstFrameIndex]);
      LastRenderTime = timeValue;
    }
  }

  // Only one animation enabled at a time
  void EnableAnimation(vtkIdType animationIndex)
  {
    // Animations are divided into groups, but stored as a vector of polydata.
    // This functions set the indices for the first and last frames in the group.
    std::size_t i = 0;
    while (i < GroupAndTimeVal.size() - 1 && GroupAndTimeVal[++i].first < animationIndex)
    {
    }
    FirstFrameIndex = animationIndex == 0 ? 0 : LastFrameIndex + 1;
    while (i < GroupAndTimeVal.size() - 1 && GroupAndTimeVal[++i].first == animationIndex)
    {
    }
    LastFrameIndex = (int)i - 1;
  }

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

  vtkQuakeMDLImporter* Parent;
  std::string Description;
  vtkSmartPointer<vtkActor> Actor;
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  std::vector<vtkSmartPointer<vtkPolyData>> Mesh;
  std::vector<std::pair<int, float>> GroupAndTimeVal;
  int ActiveAnimationId = 0;
  int CurrentFrameIndex = 0;
  int FirstFrameIndex = 0;
  int LastFrameIndex = 10;
  int NumberOfAnimations = 0;
  double LastRenderTime = 0.0;
  vtkSmartPointer<vtkTexture> Texture;
  vtkSmartPointer<vtkFloatArray> TextureCoords;
  unsigned char DefaultColorMap[256][3] = { { 0, 0, 0 }, { 15, 15, 15 }, { 31, 31, 31 },
    { 47, 47, 47 }, { 63, 63, 63 }, { 75, 75, 75 }, { 91, 91, 91 }, { 107, 107, 107 },
    { 123, 123, 123 }, { 139, 139, 139 }, { 155, 155, 155 }, { 171, 171, 171 }, { 187, 187, 187 },
    { 203, 203, 203 }, { 219, 219, 219 }, { 235, 235, 235 }, { 15, 11, 7 }, { 23, 15, 11 },
    { 31, 23, 11 }, { 39, 27, 15 }, { 47, 35, 19 }, { 55, 43, 23 }, { 63, 47, 23 }, { 75, 55, 27 },
    { 83, 59, 27 }, { 91, 67, 31 }, { 99, 75, 31 }, { 107, 83, 31 }, { 115, 87, 31 },
    { 123, 95, 35 }, { 131, 103, 35 }, { 143, 111, 35 }, { 11, 11, 15 }, { 19, 19, 27 },
    { 27, 27, 39 }, { 39, 39, 51 }, { 47, 47, 63 }, { 55, 55, 75 }, { 63, 63, 87 }, { 71, 71, 103 },
    { 79, 79, 115 }, { 91, 91, 127 }, { 99, 99, 139 }, { 107, 107, 151 }, { 115, 115, 163 },
    { 123, 123, 175 }, { 131, 131, 187 }, { 139, 139, 203 }, { 0, 0, 0 }, { 7, 7, 0 },
    { 11, 11, 0 }, { 19, 19, 0 }, { 27, 27, 0 }, { 35, 35, 0 }, { 43, 43, 7 }, { 47, 47, 7 },
    { 55, 55, 7 }, { 63, 63, 7 }, { 71, 71, 7 }, { 75, 75, 11 }, { 83, 83, 11 }, { 91, 91, 11 },
    { 99, 99, 11 }, { 107, 107, 15 }, { 7, 0, 0 }, { 15, 0, 0 }, { 23, 0, 0 }, { 31, 0, 0 },
    { 39, 0, 0 }, { 47, 0, 0 }, { 55, 0, 0 }, { 63, 0, 0 }, { 71, 0, 0 }, { 79, 0, 0 },
    { 87, 0, 0 }, { 95, 0, 0 }, { 103, 0, 0 }, { 111, 0, 0 }, { 119, 0, 0 }, { 127, 0, 0 },
    { 19, 19, 0 }, { 27, 27, 0 }, { 35, 35, 0 }, { 47, 43, 0 }, { 55, 47, 0 }, { 67, 55, 0 },
    { 75, 59, 7 }, { 87, 67, 7 }, { 95, 71, 7 }, { 107, 75, 11 }, { 119, 83, 15 }, { 131, 87, 19 },
    { 139, 91, 19 }, { 151, 95, 27 }, { 163, 99, 31 }, { 175, 103, 35 }, { 35, 19, 7 },
    { 47, 23, 11 }, { 59, 31, 15 }, { 75, 35, 19 }, { 87, 43, 23 }, { 99, 47, 31 }, { 115, 55, 35 },
    { 127, 59, 43 }, { 143, 67, 51 }, { 159, 79, 51 }, { 175, 99, 47 }, { 191, 119, 47 },
    { 207, 143, 43 }, { 223, 171, 39 }, { 239, 203, 31 }, { 255, 243, 27 }, { 11, 7, 0 },
    { 27, 19, 0 }, { 43, 35, 15 }, { 55, 43, 19 }, { 71, 51, 27 }, { 83, 55, 35 }, { 99, 63, 43 },
    { 111, 71, 51 }, { 127, 83, 63 }, { 139, 95, 71 }, { 155, 107, 83 }, { 167, 123, 95 },
    { 183, 135, 107 }, { 195, 147, 123 }, { 211, 163, 139 }, { 227, 179, 151 }, { 171, 139, 163 },
    { 159, 127, 151 }, { 147, 115, 135 }, { 139, 103, 123 }, { 127, 91, 111 }, { 119, 83, 99 },
    { 107, 75, 87 }, { 95, 63, 75 }, { 87, 55, 67 }, { 75, 47, 55 }, { 67, 39, 47 }, { 55, 31, 35 },
    { 43, 23, 27 }, { 35, 19, 19 }, { 23, 11, 11 }, { 15, 7, 7 }, { 187, 115, 159 },
    { 175, 107, 143 }, { 163, 95, 131 }, { 151, 87, 119 }, { 139, 79, 107 }, { 127, 75, 95 },
    { 115, 67, 83 }, { 107, 59, 75 }, { 95, 51, 63 }, { 83, 43, 55 }, { 71, 35, 43 },
    { 59, 31, 35 }, { 47, 23, 27 }, { 35, 19, 19 }, { 23, 11, 11 }, { 15, 7, 7 }, { 219, 195, 187 },
    { 203, 179, 167 }, { 191, 163, 155 }, { 175, 151, 139 }, { 163, 135, 123 }, { 151, 123, 111 },
    { 135, 111, 95 }, { 123, 99, 83 }, { 107, 87, 71 }, { 95, 75, 59 }, { 83, 63, 51 },
    { 67, 51, 39 }, { 55, 43, 31 }, { 39, 31, 23 }, { 27, 19, 15 }, { 15, 11, 7 },
    { 111, 131, 123 }, { 103, 123, 111 }, { 95, 115, 103 }, { 87, 107, 95 }, { 79, 99, 87 },
    { 71, 91, 79 }, { 63, 83, 71 }, { 55, 75, 63 }, { 47, 67, 55 }, { 43, 59, 47 }, { 35, 51, 39 },
    { 31, 43, 31 }, { 23, 35, 23 }, { 15, 27, 19 }, { 11, 19, 11 }, { 7, 11, 7 }, { 255, 243, 27 },
    { 239, 223, 23 }, { 219, 203, 19 }, { 203, 183, 15 }, { 187, 167, 15 }, { 171, 151, 11 },
    { 155, 131, 7 }, { 139, 115, 7 }, { 123, 99, 7 }, { 107, 83, 0 }, { 91, 71, 0 }, { 75, 55, 0 },
    { 59, 43, 0 }, { 43, 31, 0 }, { 27, 15, 0 }, { 11, 7, 0 }, { 0, 0, 255 }, { 11, 11, 239 },
    { 19, 19, 223 }, { 27, 27, 207 }, { 35, 35, 191 }, { 43, 43, 175 }, { 47, 47, 159 },
    { 47, 47, 143 }, { 47, 47, 127 }, { 47, 47, 111 }, { 47, 47, 95 }, { 43, 43, 79 },
    { 35, 35, 63 }, { 27, 27, 47 }, { 19, 19, 31 }, { 11, 11, 15 }, { 43, 0, 0 }, { 59, 0, 0 },
    { 75, 7, 0 }, { 95, 7, 0 }, { 111, 15, 0 }, { 127, 23, 7 }, { 147, 31, 7 }, { 163, 39, 11 },
    { 183, 51, 15 }, { 195, 75, 27 }, { 207, 99, 43 }, { 219, 127, 59 }, { 227, 151, 79 },
    { 231, 171, 95 }, { 239, 191, 119 }, { 247, 211, 139 }, { 167, 123, 59 }, { 183, 155, 55 },
    { 199, 195, 55 }, { 231, 227, 87 }, { 127, 191, 255 }, { 171, 231, 255 }, { 215, 255, 255 },
    { 103, 0, 0 }, { 139, 0, 0 }, { 179, 0, 0 }, { 215, 0, 0 }, { 255, 0, 0 }, { 255, 243, 147 },
    { 255, 247, 199 }, { 255, 255, 255 }, { 159, 91, 83 } };
  float NormalVectors[162][3] = { { -0.525731f, 0.000000f, 0.850651f },
    { -0.442863f, 0.238856f, 0.864188f }, { -0.295242f, 0.000000f, 0.955423f },
    { -0.309017f, 0.500000f, 0.809017f }, { -0.162460f, 0.262866f, 0.951056f },
    { 0.000000f, 0.000000f, 1.000000f }, { 0.000000f, 0.850651f, 0.525731f },
    { -0.147621f, 0.716567f, 0.681718f }, { 0.147621f, 0.716567f, 0.681718f },
    { 0.000000f, 0.525731f, 0.850651f }, { 0.309017f, 0.500000f, 0.809017f },
    { 0.525731f, 0.000000f, 0.850651f }, { 0.295242f, 0.000000f, 0.955423f },
    { 0.442863f, 0.238856f, 0.864188f }, { 0.162460f, 0.262866f, 0.951056f },
    { -0.681718f, 0.147621f, 0.716567f }, { -0.809017f, 0.309017f, 0.500000f },
    { -0.587785f, 0.425325f, 0.688191f }, { -0.850651f, 0.525731f, 0.000000f },
    { -0.864188f, 0.442863f, 0.238856f }, { -0.716567f, 0.681718f, 0.147621f },
    { -0.688191f, 0.587785f, 0.425325f }, { -0.500000f, 0.809017f, 0.309017f },
    { -0.238856f, 0.864188f, 0.442863f }, { -0.425325f, 0.688191f, 0.587785f },
    { -0.716567f, 0.681718f, -0.147621f }, { -0.500000f, 0.809017f, -0.309017f },
    { -0.525731f, 0.850651f, 0.000000f }, { 0.000000f, 0.850651f, -0.525731f },
    { -0.238856f, 0.864188f, -0.442863f }, { 0.000000f, 0.955423f, -0.295242f },
    { -0.262866f, 0.951056f, -0.162460f }, { 0.000000f, 1.000000f, 0.000000f },
    { 0.000000f, 0.955423f, 0.295242f }, { -0.262866f, 0.951056f, 0.162460f },
    { 0.238856f, 0.864188f, 0.442863f }, { 0.262866f, 0.951056f, 0.162460f },
    { 0.500000f, 0.809017f, 0.309017f }, { 0.238856f, 0.864188f, -0.442863f },
    { 0.262866f, 0.951056f, -0.162460f }, { 0.500000f, 0.809017f, -0.309017f },
    { 0.850651f, 0.525731f, 0.000000f }, { 0.716567f, 0.681718f, 0.147621f },
    { 0.716567f, 0.681718f, -0.147621f }, { 0.525731f, 0.850651f, 0.000000f },
    { 0.425325f, 0.688191f, 0.587785f }, { 0.864188f, 0.442863f, 0.238856f },
    { 0.688191f, 0.587785f, 0.425325f }, { 0.809017f, 0.309017f, 0.500000f },
    { 0.681718f, 0.147621f, 0.716567f }, { 0.587785f, 0.425325f, 0.688191f },
    { 0.955423f, 0.295242f, 0.000000f }, { 1.000000f, 0.000000f, 0.000000f },
    { 0.951056f, 0.162460f, 0.262866f }, { 0.850651f, -0.525731f, 0.000000f },
    { 0.955423f, -0.295242f, 0.000000f }, { 0.864188f, -0.442863f, 0.238856f },
    { 0.951056f, -0.162460f, 0.262866f }, { 0.809017f, -0.309017f, 0.500000f },
    { 0.681718f, -0.147621f, 0.716567f }, { 0.850651f, 0.000000f, 0.525731f },
    { 0.864188f, 0.442863f, -0.238856f }, { 0.809017f, 0.309017f, -0.500000f },
    { 0.951056f, 0.162460f, -0.262866f }, { 0.525731f, 0.000000f, -0.850651f },
    { 0.681718f, 0.147621f, -0.716567f }, { 0.681718f, -0.147621f, -0.716567f },
    { 0.850651f, 0.000000f, -0.525731f }, { 0.809017f, -0.309017f, -0.500000f },
    { 0.864188f, -0.442863f, -0.238856f }, { 0.951056f, -0.162460f, -0.262866f },
    { 0.147621f, 0.716567f, -0.681718f }, { 0.309017f, 0.500000f, -0.809017f },
    { 0.425325f, 0.688191f, -0.587785f }, { 0.442863f, 0.238856f, -0.864188f },
    { 0.587785f, 0.425325f, -0.688191f }, { 0.688191f, 0.587785f, -0.425325f },
    { -0.147621f, 0.716567f, -0.681718f }, { -0.309017f, 0.500000f, -0.809017f },
    { 0.000000f, 0.525731f, -0.850651f }, { -0.525731f, 0.000000f, -0.850651f },
    { -0.442863f, 0.238856f, -0.864188f }, { -0.295242f, 0.000000f, -0.955423f },
    { -0.162460f, 0.262866f, -0.951056f }, { 0.000000f, 0.000000f, -1.000000f },
    { 0.295242f, 0.000000f, -0.955423f }, { 0.162460f, 0.262866f, -0.951056f },
    { -0.442863f, -0.238856f, -0.864188f }, { -0.309017f, -0.500000f, -0.809017f },
    { -0.162460f, -0.262866f, -0.951056f }, { 0.000000f, -0.850651f, -0.525731f },
    { -0.147621f, -0.716567f, -0.681718f }, { 0.147621f, -0.716567f, -0.681718f },
    { 0.000000f, -0.525731f, -0.850651f }, { 0.309017f, -0.500000f, -0.809017f },
    { 0.442863f, -0.238856f, -0.864188f }, { 0.162460f, -0.262866f, -0.951056f },
    { 0.238856f, -0.864188f, -0.442863f }, { 0.500000f, -0.809017f, -0.309017f },
    { 0.425325f, -0.688191f, -0.587785f }, { 0.716567f, -0.681718f, -0.147621f },
    { 0.688191f, -0.587785f, -0.425325f }, { 0.587785f, -0.425325f, -0.688191f },
    { 0.000000f, -0.955423f, -0.295242f }, { 0.000000f, -1.000000f, 0.000000f },
    { 0.262866f, -0.951056f, -0.162460f }, { 0.000000f, -0.850651f, 0.525731f },
    { 0.000000f, -0.955423f, 0.295242f }, { 0.238856f, -0.864188f, 0.442863f },
    { 0.262866f, -0.951056f, 0.162460f }, { 0.500000f, -0.809017f, 0.309017f },
    { 0.716567f, -0.681718f, 0.147621f }, { 0.525731f, -0.850651f, 0.000000f },
    { -0.238856f, -0.864188f, -0.442863f }, { -0.500000f, -0.809017f, -0.309017f },
    { -0.262866f, -0.951056f, -0.162460f }, { -0.850651f, -0.525731f, 0.000000f },
    { -0.716567f, -0.681718f, -0.147621f }, { -0.716567f, -0.681718f, 0.147621f },
    { -0.525731f, -0.850651f, 0.000000f }, { -0.500000f, -0.809017f, 0.309017f },
    { -0.238856f, -0.864188f, 0.442863f }, { -0.262866f, -0.951056f, 0.162460f },
    { -0.864188f, -0.442863f, 0.238856f }, { -0.809017f, -0.309017f, 0.500000f },
    { -0.688191f, -0.587785f, 0.425325f }, { -0.681718f, -0.147621f, 0.716567f },
    { -0.442863f, -0.238856f, 0.864188f }, { -0.587785f, -0.425325f, 0.688191f },
    { -0.309017f, -0.500000f, 0.809017f }, { -0.147621f, -0.716567f, 0.681718f },
    { -0.425325f, -0.688191f, 0.587785f }, { -0.162460f, -0.262866f, 0.951056f },
    { 0.442863f, -0.238856f, 0.864188f }, { 0.162460f, -0.262866f, 0.951056f },
    { 0.309017f, -0.500000f, 0.809017f }, { 0.147621f, -0.716567f, 0.681718f },
    { 0.000000f, -0.525731f, 0.850651f }, { 0.425325f, -0.688191f, 0.587785f },
    { 0.587785f, -0.425325f, 0.688191f }, { 0.688191f, -0.587785f, 0.425325f },
    { -0.955423f, 0.295242f, 0.000000f }, { -0.951056f, 0.162460f, 0.262866f },
    { -1.000000f, 0.000000f, 0.000000f }, { -0.850651f, 0.000000f, 0.525731f },
    { -0.955423f, -0.295242f, 0.000000f }, { -0.951056f, -0.162460f, 0.262866f },
    { -0.864188f, 0.442863f, -0.238856f }, { -0.951056f, 0.162460f, -0.262866f },
    { -0.809017f, 0.309017f, -0.500000f }, { -0.864188f, -0.442863f, -0.238856f },
    { -0.951056f, -0.162460f, -0.262866f }, { -0.809017f, -0.309017f, -0.500000f },
    { -0.681718f, 0.147621f, -0.716567f }, { -0.681718f, -0.147621f, -0.716567f },
    { -0.850651f, 0.000000f, -0.525731f }, { -0.688191f, 0.587785f, -0.425325f },
    { -0.587785f, 0.425325f, -0.688191f }, { -0.425325f, 0.688191f, -0.587785f },
    { -0.425325f, -0.688191f, -0.587785f }, { -0.587785f, -0.425325f, -0.688191f },
    { -0.688191f, -0.587785f, -0.425325f } };
};

vtkQuakeMDLImporter::vtkQuakeMDLImporter()
  : Internals(new vtkQuakeMDLImporter::vtkInternals(this)){

  };

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
int vtkQuakeMDLImporter::ImportBegin()
{
  return this->Internals->ReadScene(this->FileName);
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::ImportActors(vtkRenderer* renderer)
{
  this->Internals->ImportActors(renderer);
}

//----------------------------------------------------------------------------
std::string vtkQuakeMDLImporter::GetOutputsDescription()
{
  return this->Internals->Description;
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::UpdateTimeStep(double timeValue)
{
  this->Internals->UpdateFrame(timeValue);
  return;
}

//----------------------------------------------------------------------------
vtkIdType vtkQuakeMDLImporter::GetNumberOfAnimations()
{
  return this->Internals->NumberOfAnimations;
}

//----------------------------------------------------------------------------
std::string vtkQuakeMDLImporter::GetAnimationName(vtkIdType animationIndex)
{
  return std::to_string(animationIndex);
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::EnableAnimation(vtkIdType animationIndex)
{
  this->Internals->EnableAnimation(animationIndex);
  return;
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::DisableAnimation(vtkIdType vtkNotUsed(animationIndex))
{
  return;
}

//----------------------------------------------------------------------------
bool vtkQuakeMDLImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  return animationIndex == this->Internals->ActiveAnimationId;
}

//----------------------------------------------------------------------------
bool vtkQuakeMDLImporter::GetTemporalInformation(vtkIdType vtkNotUsed(animationIndex),
  double vtkNotUsed(frameRate), int& vtkNotUsed(nbTimeSteps), double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  timeRange[0] = 0.0;
  timeRange[1] = 10.0;
  return true;
}

//----------------------------------------------------------------------------
vtkIdType vtkQuakeMDLImporter::GetNumberOfCameras()
{
  return 1;
}

//----------------------------------------------------------------------------
std::string vtkQuakeMDLImporter::GetCameraName(vtkIdType vtkNotUsed(camIndex))
{
  return "Camera";
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::SetCamera(vtkIdType vtkNotUsed(camIndex))
{
  return;
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::ImportCameras(vtkRenderer* renderer)
{
  this->Internals->ImportCameras(renderer);
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::ImportLights(vtkRenderer* renderer)
{
  this->Internals->ImportLights(renderer);
}

//----------------------------------------------------------------------------
void vtkQuakeMDLImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
