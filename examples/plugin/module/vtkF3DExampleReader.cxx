#include "vtkF3DExampleReader.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DExampleReader);

//----------------------------------------------------------------------------
vtkF3DExampleReader::vtkF3DExampleReader()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DExampleReader::~vtkF3DExampleReader() = default;

//----------------------------------------------------------------------------
void vtkF3DExampleReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << (this->FileName.empty() ? "(none)" : this->FileName) << "\n";
}

//----------------------------------------------------------------------------
int vtkF3DExampleReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  std::ifstream f(this->FileName);

  if (f.is_open())
  {
    std::vector<std::string> tokens;
    std::copy(std::istream_iterator<std::string>(f), std::istream_iterator<std::string>(),
      std::back_inserter(tokens));

    if (tokens.size() % 3 != 0)
    {
      vtkErrorMacro("Found " << tokens.size() << " numbers, which is not a multiple of 3.");
      return 0;
    }

    vtkNew<vtkPoints> points;

    for (size_t i = 0; i < tokens.size(); i += 3)
    {
      points->InsertNextPoint(std::atof(tokens[i].c_str()), std::atof(tokens[i + 1].c_str()),
        std::atof(tokens[i + 2].c_str()));
    }

    output->SetPoints(points);

    f.close();
  }
  else
  {
    vtkErrorMacro("File " << this->FileName << " cannot be found.");
    return 0;
  }

  return 1;
}
