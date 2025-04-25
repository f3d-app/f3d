#include "vtkF3DSPZReader.h"

#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <load-spz.h>

class vtkF3DSPZReader::vtkInternals
{
};

vtkStandardNewMacro(vtkF3DSPZReader);

//----------------------------------------------------------------------------
vtkF3DSPZReader::vtkF3DSPZReader()
  : Internals(new vtkF3DSPZReader::vtkInternals())
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DSPZReader::~vtkF3DSPZReader() = default;

//----------------------------------------------------------------------------
int vtkF3DSPZReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  spz::GaussianCloud gaussians = spz::loadSpz(this->FileName, { spz::CoordinateSystem::RDF });

  int nbSplats = gaussians.numPoints;

  vtkNew<vtkFloatArray> positionArray;
  positionArray->SetNumberOfComponents(3);
  positionArray->SetNumberOfTuples(nbSplats);
  positionArray->SetName("position");

  vtkNew<vtkFloatArray> scaleArray;
  scaleArray->SetNumberOfComponents(3);
  scaleArray->SetNumberOfTuples(nbSplats);
  scaleArray->SetName("scale");

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(nbSplats);
  colorArray->SetName("color");

  vtkNew<vtkFloatArray> rotationArray;
  rotationArray->SetNumberOfComponents(4);
  rotationArray->SetNumberOfTuples(nbSplats);
  rotationArray->SetName("rotation");

  auto sigmoid = [](float x) -> float { return 1.f / (1.f + std::exp(-x)); };

  for (size_t i = 0; i < nbSplats; i++)
  {
    positionArray->SetTypedTuple(i, gaussians.positions.data() + 3 * i);

    for (int c = 0; c < 3; c++)
    {
      constexpr float SH_C0 = 0.28209479177387814;
      unsigned char col = static_cast<unsigned char>((gaussians.colors[3 * i + c] * SH_C0 + 0.5) * 255.0);
      colorArray->SetTypedComponent(i, c, col);

      scaleArray->SetTypedComponent(i, c, std::exp(gaussians.scales[3 * i + c]));

      rotationArray->SetTypedComponent(i, c, gaussians.rotations[4 * i + 1 + c]);
    }
    colorArray->SetTypedComponent(i, 3, static_cast<unsigned char>(sigmoid(gaussians.alphas[i]) * 255.0));
    rotationArray->SetTypedComponent(i, 3, gaussians.rotations[4 * i]);
  }

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(positionArray);
  output->SetPoints(points);

  output->GetPointData()->SetScalars(colorArray);
  output->GetPointData()->AddArray(scaleArray);
  output->GetPointData()->AddArray(rotationArray);

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DSPZReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
