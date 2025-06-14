#include "vtkF3DPostProcessFilter.h"

#include "F3DLog.h"

#include <vtkAppendPolyData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataObject.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkImageData.h>
#include <vtkImageToPoints.h>
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridToPointSet.h>
#include <vtkResampleToImage.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertexGlyphFilter.h>

#include <numeric>

vtkStandardNewMacro(vtkF3DPostProcessFilter);

//----------------------------------------------------------------------------
vtkF3DPostProcessFilter::vtkF3DPostProcessFilter()
{
  this->SetNumberOfOutputPorts(3);
}

//----------------------------------------------------------------------------
int vtkF3DPostProcessFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkDataObject* dataObject = vtkDataObject::GetData(inputVector[0]);
  vtkPolyData* outputSurface = vtkPolyData::GetData(outputVector, 0);
  vtkPolyData* outputPoints = vtkPolyData::GetData(outputVector, 1);
  vtkImageData* outputImage = vtkImageData::GetData(outputVector, 2);

  vtkCompositeDataSet* composite = vtkCompositeDataSet::SafeDownCast(dataObject);
  vtkSmartPointer<vtkDataSet> dataset = vtkDataSet::SafeDownCast(dataObject);

  // Extract data from a composite dataset
  if (composite)
  {
    auto iter = vtkSmartPointer<vtkCompositeDataIterator>::Take(composite->NewIterator());
    iter->SkipEmptyNodesOn();

    // If it contains a single leaf, extract it as is
    int nLeaf = 0;
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
      dataset = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      nLeaf++;
    }

    // If multiple leaves, extract all surfaces and append them together
    if (nLeaf > 1)
    {
      vtkNew<vtkAppendPolyData> append;
      for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
      {
        vtkDataSet* leafDS = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
        vtkSmartPointer<vtkPolyData> leafPD = vtkPolyData::SafeDownCast(leafDS);
        if (!leafDS)
        {
          F3DLog::Print(F3DLog::Severity::Warning,
            "A non data set block was ignored while reading a composite.");
        }
        else
        {
          if (!leafPD)
          {
            vtkNew<vtkDataSetSurfaceFilter> geom;
            geom->SetInputData(iter->GetCurrentDataObject());
            geom->Update();
            leafPD = vtkPolyData::SafeDownCast(geom->GetOutput());
          }
          append->AddInputData(leafPD);
        }
      }

      append->Update();
      dataset = append->GetOutput();
    }
  }

  // If the input is a polydata or an unstructured grid without cells, add a polyvertex cell
  vtkPolyData* pd = vtkPolyData::SafeDownCast(dataset);
  vtkUnstructuredGrid* ug = vtkUnstructuredGrid::SafeDownCast(dataset);
  if ((pd || ug) && dataset->GetNumberOfCells() == 0)
  {
    std::vector<vtkIdType> polyVertex(dataset->GetNumberOfPoints());
    std::iota(polyVertex.begin(), polyVertex.end(), 0);
    if (pd)
    {
      vtkNew<vtkCellArray> verts;
      verts->InsertNextCell(pd->GetNumberOfPoints(), polyVertex.data());
      pd->SetVerts(verts);
    }
    else if (ug)
    {
      ug->InsertNextCell(VTK_POLY_VERTEX, ug->GetNumberOfPoints(), polyVertex.data());
    }
  }

  // Check if input is an image
  vtkImageData* image = vtkImageData::SafeDownCast(dataset);
  if (image)
  {
    outputImage->ShallowCopy(image);
  }

  // Recover the surface of the dataset if not available already
  // Recover a cloud of pointer for the cloud output as well
  vtkSmartPointer<vtkPolyData> surface = vtkPolyData::SafeDownCast(dataset);
  vtkSmartPointer<vtkPolyData> cloud = surface;
  if (!surface)
  {
    vtkNew<vtkDataSetSurfaceFilter> geom;
    geom->SetInputData(dataset);
    geom->Update();
    surface = vtkPolyData::SafeDownCast(geom->GetOutput());

    if (image)
    {
      vtkNew<vtkImageToPoints> imageCloudFilter;
      imageCloudFilter->SetInputData(dataset);
      imageCloudFilter->Update();
      cloud = vtkPolyData::SafeDownCast(imageCloudFilter->GetOutput());
    }
    else if (vtkRectilinearGrid::SafeDownCast(dataset))
    {
      vtkNew<vtkRectilinearGridToPointSet> pointSetFilter;
      pointSetFilter->SetInputData(dataset);
      vtkNew<vtkVertexGlyphFilter> vertexFilter;
      vertexFilter->SetInputConnection(pointSetFilter->GetOutputPort());
      vertexFilter->Update();
      cloud = vtkPolyData::SafeDownCast(vertexFilter->GetOutput());
    }
    else if (vtkPointSet::SafeDownCast(dataset))
    {
      vtkNew<vtkVertexGlyphFilter> vertexFilter;
      vertexFilter->SetInputData(dataset);
      vertexFilter->Update();
      cloud = vtkPolyData::SafeDownCast(vertexFilter->GetOutput());
    }
  }

  outputSurface->ShallowCopy(surface);
  outputPoints->ShallowCopy(cloud);

  return 1;
}

//----------------------------------------------------------------------------
int vtkF3DPostProcessFilter::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCompositeDataSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkF3DPostProcessFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (port < 2)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  }
  else
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
  }
  return 1;
}
