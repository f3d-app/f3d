#include "vtkF3DOpen3SDCMImporter.h"

#include "ParseDcm.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkResourceStream.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkUnsignedCharArray.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

vtkStandardNewMacro(vtkF3DOpen3SDCMImporter);

//----------------------------------------------------------------------------
class vtkF3DOpen3SDCMImporter::vtkInternals
{
public:
  explicit vtkInternals(vtkF3DOpen3SDCMImporter* parent)
    : Parent(parent)
  {
  }

  ~vtkInternals() = default;

  // Parse the DCM file and store the data
  bool ParseFile(const std::string& filename);

  // Create a vtkTexture from embedded texture image data
  vtkSmartPointer<vtkTexture> CreateTextureFromEmbeddedImage(
    const Open3SDCM::EmbeddedTextureImage& textureImage);

  // Create vtkPolyData from parsed mesh data
  vtkSmartPointer<vtkPolyData> CreatePolyData();

  // Create vtkProperty with texture if available
  vtkSmartPointer<vtkProperty> CreateProperty();

  vtkF3DOpen3SDCMImporter* Parent;
  Open3SDCM::DCMParser Parser;
  vtkSmartPointer<vtkPolyData> PolyData;
  std::vector<vtkSmartPointer<vtkTexture>> Textures;
  std::string Description;
};

//----------------------------------------------------------------------------
bool vtkF3DOpen3SDCMImporter::vtkInternals::ParseFile(const std::string& filename)
{
  try
  {
    this->Parser.ParseDCM(filename);
  }
  catch (const std::exception& e)
  {
    vtkErrorWithObjectMacro(this->Parent, "Failed to parse DCM file: " << e.what());
    return false;
  }

  // Check if we got valid data
  if (this->Parser.m_Vertices.empty() || this->Parser.m_Triangles.empty())
  {
    vtkErrorWithObjectMacro(this->Parent, "No mesh data found in DCM file: " << filename);
    return false;
  }

  // Build description
  const size_t numVertices = this->Parser.m_Vertices.size() / 3;
  const size_t numTriangles = this->Parser.m_Triangles.size();
  this->Description = "3Shape DCM file\n";
  this->Description += "Number of points: ";
  this->Description += std::to_string(numVertices);
  this->Description += "\n";
  this->Description += "Number of cells: ";
  this->Description += std::to_string(numTriangles);
  this->Description += "\n";

  if (!this->Parser.m_SurfaceData.textureImages.empty())
  {
    this->Description += "Embedded textures: ";
    this->Description += std::to_string(this->Parser.m_SurfaceData.textureImages.size());
    this->Description += "\n";
  }

  return true;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkTexture> vtkF3DOpen3SDCMImporter::vtkInternals::CreateTextureFromEmbeddedImage(
  const Open3SDCM::EmbeddedTextureImage& textureImage)
{
  vtkNew<vtkTexture> texture;

  // Check if we have valid image data
  if (textureImage.imageBytes.empty() || textureImage.width == 0 || textureImage.height == 0)
  {
    vtkWarningWithObjectMacro(this->Parent, "Invalid embedded texture image data");
    return nullptr;
  }

  // Create vtkImageData based on the texture format
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(static_cast<int>(textureImage.width), 
                          static_cast<int>(textureImage.height), 1);

  // Determine the number of components based on bytesPerPixel
  int numComponents = static_cast<int>(textureImage.bytesPerPixel);
  
  // Handle common cases:
  // - 3 bytes: RGB (3 components)
  // - 4 bytes: RGBA (4 components)
  // - 1 byte: Grayscale (1 component, but we'll use 3 for compatibility)
  if (numComponents == 1)
  {
    // Grayscale - convert to RGB by replicating the value
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    
    unsigned char* outPtr = static_cast<unsigned char*>(imageData->GetScalarPointer());
    for (size_t i = 0; i < textureImage.imageBytes.size(); ++i)
    {
      unsigned char value = textureImage.imageBytes[i];
      *outPtr++ = value; // R
      *outPtr++ = value; // G
      *outPtr++ = value; // B
    }
  }
  else if (numComponents == 3)
  {
    // RGB
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    std::copy(textureImage.imageBytes.begin(), textureImage.imageBytes.end(),
              static_cast<unsigned char*>(imageData->GetScalarPointer()));
  }
  else if (numComponents == 4)
  {
    // RGBA
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
    std::copy(textureImage.imageBytes.begin(), textureImage.imageBytes.end(),
              static_cast<unsigned char*>(imageData->GetScalarPointer()));
  }
  else
  {
    // Unknown format - try to use as-is
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, numComponents);
    std::copy(textureImage.imageBytes.begin(), textureImage.imageBytes.end(),
              static_cast<unsigned char*>(imageData->GetScalarPointer()));
  }

  texture->SetInputData(imageData);
  texture->MipmapOn();
  texture->InterpolateOn();
  texture->SetColorModeToDirectScalars();

  return texture;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkF3DOpen3SDCMImporter::vtkInternals::CreatePolyData()
{
  const size_t numVertices = this->Parser.m_Vertices.size() / 3;
  
  if (numVertices == 0)
  {
    return nullptr;
  }

  vtkNew<vtkPolyData> polyData;

  // Create points
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetNumberOfPoints(static_cast<vtkIdType>(numVertices));

  float* ptr = static_cast<float*>(points->GetVoidPointer(0));
  std::copy(this->Parser.m_Vertices.begin(), this->Parser.m_Vertices.end(), ptr);
  polyData->SetPoints(points);

  // Create cells (triangles)
  vtkNew<vtkCellArray> cells;

  for (const auto& tri : this->Parser.m_Triangles)
  {
    cells->InsertNextCell(3);
    cells->InsertCellPoint(static_cast<vtkIdType>(tri.v1));
    cells->InsertCellPoint(static_cast<vtkIdType>(tri.v2));
    cells->InsertCellPoint(static_cast<vtkIdType>(tri.v3));
  }
  polyData->SetPolys(cells);

  // Add colors if available
  if (this->Parser.m_SurfaceData.baseColor)
  {
    const auto& color = *this->Parser.m_SurfaceData.baseColor;
    
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetName("Colors");
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(points->GetNumberOfPoints());

    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
      colors->SetTuple3(i, color.r, color.g, color.b);
    }

    polyData->GetPointData()->SetScalars(colors);
  }

  // Add texture coordinates if available
  if (!this->Parser.m_SurfaceData.textureCoordinates.empty())
  {
    const auto& texCoordData = this->Parser.m_SurfaceData.textureCoordinates[0];
    
    // Check if we have decoded texture coordinates
    if (texCoordData.HasDecodedCoordinates() && 
        texCoordData.cornerCoordinates.size() == this->Parser.m_Triangles.size() * 3)
    {
      // Texture coordinates are per-corner (3 per triangle)
      // We need to convert them to per-vertex coordinates
      // For each vertex, we'll use the average of all its texture coordinates
      
      vtkNew<vtkFloatArray> tcoords;
      tcoords->SetName("TCoords");
      tcoords->SetNumberOfComponents(2);
      tcoords->SetNumberOfTuples(numVertices);
      
      // Initialize all to (0,0)
      for (vtkIdType i = 0; i < numVertices; ++i)
      {
        tcoords->SetTypedTuple(i, std::array<float, 2>{0.0f, 0.0f}.data());
      }
      
      // Build a map from vertex index to all its texture coordinates
      std::vector<std::vector<std::pair<size_t, Open3SDCM::TextureCoordinate>>> vertexTexCoords(numVertices);
      
      for (size_t triIdx = 0; triIdx < this->Parser.m_Triangles.size(); ++triIdx)
      {
        const auto& tri = this->Parser.m_Triangles[triIdx];
        const size_t cornerIdx = triIdx * 3;
        
        if (texCoordData.cornerCoordinates[cornerIdx].has_value())
        {
          vertexTexCoords[tri.v1].emplace_back(cornerIdx, *texCoordData.cornerCoordinates[cornerIdx]);
        }
        if (texCoordData.cornerCoordinates[cornerIdx + 1].has_value())
        {
          vertexTexCoords[tri.v2].emplace_back(cornerIdx + 1, *texCoordData.cornerCoordinates[cornerIdx + 1]);
        }
        if (texCoordData.cornerCoordinates[cornerIdx + 2].has_value())
        {
          vertexTexCoords[tri.v3].emplace_back(cornerIdx + 2, *texCoordData.cornerCoordinates[cornerIdx + 2]);
        }
      }
      
      // Average texture coordinates for each vertex
      for (vtkIdType v = 0; v < numVertices; ++v)
      {
        if (!vertexTexCoords[v].empty())
        {
          float sumU = 0.0f;
          float sumV = 0.0f;
          for (const auto& [cornerIdx, texCoord] : vertexTexCoords[v])
          {
            sumU += texCoord.u;
            sumV += texCoord.v;
          }
          float avgU = sumU / vertexTexCoords[v].size();
          float avgV = sumV / vertexTexCoords[v].size();
          tcoords->SetTypedTuple(v, std::array<float, 2>{avgU, avgV}.data());
        }
      }
      
      polyData->GetPointData()->SetTCoords(tcoords);
    }
  }

  return polyData;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkProperty> vtkF3DOpen3SDCMImporter::vtkInternals::CreateProperty()
{
  vtkNew<vtkProperty> property;

  // Set base color if available
  if (this->Parser.m_SurfaceData.baseColor)
  {
    const auto& color = *this->Parser.m_SurfaceData.baseColor;
    property->SetColor(color.r / 255.0, color.g / 255.0, color.b / 255.0);
  }

  // Create and set textures from embedded images
  // Use the first texture as the base color texture
  if (!this->Parser.m_SurfaceData.textureImages.empty())
  {
    const auto& textureImage = this->Parser.m_SurfaceData.textureImages[0];
    
    vtkSmartPointer<vtkTexture> texture = this->CreateTextureFromEmbeddedImage(textureImage);
    if (texture)
    {
      this->Textures.push_back(texture);
      // Set as base color texture for diffuse mapping
      property->SetBaseColorTexture(texture);
      
      // Enable sRGB if the texture is a standard image format
      if (textureImage.mimeType && 
          (textureImage.mimeType->find("jpeg") != std::string::npos ||
           textureImage.mimeType->find("png") != std::string::npos))
      {
        texture->UseSRGBColorSpaceOn();
      }
    }
  }

  return property;
}

//----------------------------------------------------------------------------
vtkF3DOpen3SDCMImporter::vtkF3DOpen3SDCMImporter()
  : Internals(std::make_unique<vtkF3DOpen3SDCMImporter::vtkInternals>(this))
{
}

//----------------------------------------------------------------------------
vtkF3DOpen3SDCMImporter::~vtkF3DOpen3SDCMImporter() = default;

//----------------------------------------------------------------------------
bool vtkF3DOpen3SDCMImporter::CanReadFile(const std::string& filename)
{
  // Check file extension
  std::string ext;
  size_t pos = filename.find_last_of(".");
  if (pos != std::string::npos)
  {
    ext = filename.substr(pos + 1);
  }
  
  // Check for .dcm extension
  if (ext != "dcm")
  {
    return false;
  }

  // Check for DICOM header - explicitly reject DICOM files
  // DICOM files start with "DICM"
  std::ifstream file(filename, std::ios::binary);
  if (!file)
  {
    return false;
  }

  char header[16];
  if (!file.read(header, 16))
  {
    return false;
  }

  // DICOM files start with "DICM"
  const char dicomHeader[] = { 'D', 'I', 'C', 'M' };
  if (memcmp(header, dicomHeader, 4) == 0)
  {
    return false; // It's DICOM, not 3Shape
  }

  // 3Shape DCM files start with "<HPS version=" or "<HPS"
  const char hpsHeaderFull[] = "<HPS version=";
  const char hpsHeaderShort[] = "<HPS";
  if (memcmp(header, hpsHeaderFull, 12) == 0 || memcmp(header, hpsHeaderShort, 5) == 0)
  {
    return true; // It's 3Shape DCM
  }

  // Some older 3Shape DCM files might be ZIP archives containing HPS XML
  const char zipHeader[] = { 'P', 'K', '\x03', '\x04' };
  if (memcmp(header, zipHeader, 4) == 0)
  {
    return true; // It's a ZIP, could be 3Shape DCM
  }

  // Unknown format - let other readers try
  return false;
}

//----------------------------------------------------------------------------
bool vtkF3DOpen3SDCMImporter::CanReadStream(vtkResourceStream* vtkNotUsed(stream))
{
  // Stream reading is currently not supported because Open3SDCM requires file paths
  // and uses Poco::Zip which works with files, not streams.
  return false;
}

//----------------------------------------------------------------------------
int vtkF3DOpen3SDCMImporter::ImportBegin()
{
  const char* filename = this->GetFileName();
  
  if (!filename || !filename[0])
  {
    vtkErrorMacro("No filename specified");
    return 0;
  }

  if (!CanReadFile(filename))
  {
    vtkErrorMacro("File " << filename << " is not a valid 3Shape DCM file");
    return 0;
  }

  // Parse the file
  if (!this->Internals->ParseFile(filename))
  {
    return 0;
  }

  // Create the polydata
  this->Internals->PolyData = this->Internals->CreatePolyData();
  
  if (!this->Internals->PolyData)
  {
    vtkErrorMacro("Failed to create polydata from DCM file");
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DOpen3SDCMImporter::ImportActors(vtkRenderer* renderer)
{
  if (!this->Internals->PolyData)
  {
    return;
  }

  // Create mapper
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(this->Internals->PolyData);
  mapper->SetColorModeToDirectScalars();

  // Create actor
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  // Create and set property with textures
  vtkSmartPointer<vtkProperty> property = this->Internals->CreateProperty();
  actor->SetProperty(property);

  // Add actor to renderer
  renderer->AddActor(actor);
}

//----------------------------------------------------------------------------
std::string vtkF3DOpen3SDCMImporter::GetOutputsDescription()
{
  return this->Internals->Description;
}
