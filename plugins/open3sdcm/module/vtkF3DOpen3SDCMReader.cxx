#include "vtkF3DOpen3SDCMReader.h"

#include "ParseDcm.h"

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

#include <fstream>
#include <memory>

vtkStandardNewMacro(vtkF3DOpen3SDCMReader);

//----------------------------------------------------------------------------
vtkF3DOpen3SDCMReader::vtkF3DOpen3SDCMReader()
  : FileName(nullptr)
  , ResourceStream(nullptr)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkF3DOpen3SDCMReader::~vtkF3DOpen3SDCMReader()
{
  this->SetFileName(nullptr);
  this->SetResourceStream(nullptr);
}

//----------------------------------------------------------------------------
bool vtkF3DOpen3SDCMReader::CanReadFile(const std::string& filename)
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
bool vtkF3DOpen3SDCMReader::CanReadStream(vtkResourceStream* vtkNotUsed(stream))
{
  // Stream reading is currently not supported because Open3SDCM requires file paths
  // and uses Poco::Zip which works with files, not streams.
  return false;
}

//----------------------------------------------------------------------------
int vtkF3DOpen3SDCMReader::RequestInformation(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  // Set output type
  vtkInformation* outInfo = vtkInformationVector::GetOutputInformation(outputVector, 0);
  outInfo->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

//----------------------------------------------------------------------------
int vtkF3DOpen3SDCMReader::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  if (!output)
  {
    vtkErrorMacro("No output polydata");
    return 0;
  }

  // Check if we have a filename
  if (this->ResourceStream)
  {
    // Stream reading is not supported
    vtkErrorMacro("Stream reading is not supported for 3Shape DCM files");
    return 0;
  }

  if (!this->FileName || !this->FileName[0])
  {
    vtkErrorMacro("No filename specified");
    return 0;
  }

  // Check if the file is readable by this reader
  if (!CanReadFile(this->FileName))
  {
    vtkErrorMacro("File " << this->FileName << " is not a valid 3Shape DCM file");
    return 0;
  }

  // Parse the DCM file using Open3SDCM
  Open3SDCM::DCMParser parser;
  
  try
  {
    parser.ParseDCM(this->FileName);
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro("Failed to parse DCM file: " << e.what());
    return 0;
  }

  // Check if we got valid data
  if (parser.m_Vertices.empty() || parser.m_Triangles.empty())
  {
    vtkErrorMacro("No mesh data found in DCM file: " << this->FileName);
    return 0;
  }

  // Validate that we have a valid number of vertices
  const size_t numVertices = parser.m_Vertices.size() / 3;
  if (numVertices == 0)
  {
    vtkErrorMacro("No vertices found in DCM file");
    return 0;
  }

  // Validate triangle indices
  for (size_t i = 0; i < parser.m_Triangles.size(); ++i)
  {
    const auto& tri = parser.m_Triangles[i];
    if (tri.v1 >= numVertices || tri.v2 >= numVertices || tri.v3 >= numVertices)
    {
      vtkErrorMacro("Invalid triangle indices in DCM file at index " << i
                   << ": (" << tri.v1 << ", " << tri.v2 << ", " << tri.v3 
                   << ") with only " << numVertices << " vertices");
      return 0;
    }
  }

  // Create points
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetNumberOfPoints(static_cast<vtkIdType>(numVertices));

  // Copy vertices directly to VTK points
  float* ptr = static_cast<float*>(points->GetVoidPointer(0));
  std::copy(parser.m_Vertices.begin(), parser.m_Vertices.end(), ptr);

  output->SetPoints(points);

  // Create cells (triangles)
  vtkNew<vtkCellArray> cells;
  cells->SetNumberOfCells(static_cast<vtkIdType>(parser.m_Triangles.size()));

  for (const auto& tri : parser.m_Triangles)
  {
    cells->InsertNextCell(3);
    cells->InsertCellPoint(static_cast<vtkIdType>(tri.v1));
    cells->InsertCellPoint(static_cast<vtkIdType>(tri.v2));
    cells->InsertCellPoint(static_cast<vtkIdType>(tri.v3));
  }

  output->SetPolys(cells);

  // Add colors if available
  if (parser.m_SurfaceData.baseColor)
  {
    const auto& color = *parser.m_SurfaceData.baseColor;
    
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetName("Colors");
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(points->GetNumberOfPoints());

    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
      colors->SetTuple3(i, color.r, color.g, color.b);
    }

    output->GetPointData()->SetScalars(colors);
  }

  // TODO: Add support for per-vertex texture coordinates
  // TODO: Add support for embedded texture images

  return 1;
}
