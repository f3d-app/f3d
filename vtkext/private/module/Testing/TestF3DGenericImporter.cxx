#include <vtkDoubleArray.h>
#include <vtkGLTFReader.h>
#include <vtkNew.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTable.h>
#include <vtkTestUtilities.h>
#include <vtkTrivialProducer.h>
#include <vtkVersion.h>
#include <vtkXMLMultiBlockDataReader.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporter(int argc, char* argv[])
{
  vtkNew<vtkF3DGenericImporter> importer;
  if (importer->GetAnimationSupportLevel() != vtkF3DImporter::AnimationSupportLevel::UNIQUE)
  {
    std::cerr << "Unexpected animation support level\n";
    return EXIT_FAILURE;
  }
  if (importer->GetNumberOfAnimations() != 0)
  {
    std::cerr << "Unexpected number of animations\n";
    return EXIT_FAILURE;
  }

  int nbTimeSteps;
  double timeRange[2];
  vtkNew<vtkDoubleArray> timeSteps;

  if (importer->GetTemporalInformation(0, timeRange, nbTimeSteps, timeSteps))
  {
    std::cerr << "Unexpected return value with GetTemporalInformation\n";
    return EXIT_FAILURE;
  }

  // Test with reader
  vtkNew<vtkGLTFReader> reader;

  std::string filename = std::string(argv[1]) + "data/BoxAnimated.gltf";
  reader->SetFileName(filename.c_str());
  reader->UpdateInformation();
  reader->EnableAnimation(0);

  importer->SetInternalReader(reader);
  importer->Update();
  importer->Print(std::cout);
  if (importer->GetNumberOfAnimations() != 1)
  {
    std::cerr << "Unexpected number of animations\n";
    return EXIT_FAILURE;
  }

  importer->EnableAnimation(0);
  if (!importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected not enabled animation\n";
    return EXIT_FAILURE;
  }

  importer->DisableAnimation(0);
  if (importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected enabled animation\n";
    return EXIT_FAILURE;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240910)
  filename = std::string(argv[1]) + "data/BoxAnimated_invalid_animation.gltf";
  reader->SetFileName(filename.c_str());
  reader->UpdateInformation();
  reader->EnableAnimation(0);

  importer->SetInternalReader(reader);
  importer->Update();
  if (!importer->UpdateAtTimeValue(0.1))
  {
    std::cerr << "Unexpected UpdateAtTimeValue failure\n";
    return EXIT_FAILURE;
  }
#endif

  // Static method testing
  if (vtkF3DGenericImporter::GetDataObjectDescription(nullptr) != "")
  {
    std::cerr << "Unexpected data object description with null input\n";
    return EXIT_FAILURE;
  }

  // Composite/multiblock support testing
  vtkNew<vtkXMLMultiBlockDataReader> mbReader;
  std::string mbFilename = std::string(argv[1]) + "data/mb.vtm";
  mbReader->SetFileName(mbFilename.c_str());

  vtkNew<vtkF3DGenericImporter> mbImporter;
  mbImporter->SetInternalReader(mbReader);
  mbImporter->Update();

  // mb.vtm contains 3 dataset blocks (1 vtkUnstructuredGrid, 2 vtkPolyData)
  // vtkTable blocks are skipped as they are not vtkDataSet
  vtkIdType expectedBlocks = 3;
  if (mbImporter->GetNumberOfBlocks() != expectedBlocks)
  {
    std::cerr << "Unexpected number of blocks: " << mbImporter->GetNumberOfBlocks() << " (expected "
              << expectedBlocks << ")\n";
    return EXIT_FAILURE;
  }

  for (vtkIdType i = 0; i < expectedBlocks; i++)
  {
    const vtkPolyData* points = mbImporter->GetImportedPoints(i);
    if (!points)
    {
      std::cerr << "GetImportedPoints(" << i << ") returned nullptr\n";
      return EXIT_FAILURE;
    }

    std::string blockName = mbImporter->GetBlockName(i);
    if (blockName.empty())
    {
      std::cerr << "GetBlockName(" << i << ") returned empty string\n";
      return EXIT_FAILURE;
    }
  }

  if (mbImporter->GetImportedPoints() != mbImporter->GetImportedPoints(0))
  {
    std::cerr << "GetImportedPoints() doesn't match GetImportedPoints(0)\n";
    return EXIT_FAILURE;
  }

  if (mbImporter->GetImportedPoints(-1) != nullptr)
  {
    std::cerr << "GetImportedPoints(-1) should return nullptr\n";
    return EXIT_FAILURE;
  }
  if (mbImporter->GetImportedPoints(expectedBlocks) != nullptr)
  {
    std::cerr << "GetImportedPoints(expectedBlocks) should return nullptr\n";
    return EXIT_FAILURE;
  }

  if (mbImporter->GetImportedImage() != nullptr)
  {
    std::cerr << "GetImportedImage() should return nullptr for non-image data\n";
    return EXIT_FAILURE;
  }

  if (mbImporter->GetImportedImage(-1) != nullptr)
  {
    std::cerr << "GetImportedImage(-1) should return nullptr\n";
    return EXIT_FAILURE;
  }
  if (mbImporter->GetImportedImage(expectedBlocks) != nullptr)
  {
    std::cerr << "GetImportedImage(expectedBlocks) should return nullptr\n";
    return EXIT_FAILURE;
  }

  if (!mbImporter->GetBlockName(-1).empty())
  {
    std::cerr << "GetBlockName(-1) should return empty string\n";
    return EXIT_FAILURE;
  }
  if (!mbImporter->GetBlockName(expectedBlocks).empty())
  {
    std::cerr << "GetBlockName(expectedBlocks) should return empty string\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->Update();

  vtkNew<vtkPartitionedDataSet> pds;
  pds->SetNumberOfPartitions(1);
  pds->SetPartition(0, sphereSource->GetOutput());

  vtkNew<vtkPartitionedDataSetCollection> pdc;
  pdc->SetNumberOfPartitionedDataSets(1);
  pdc->SetPartitionedDataSet(0, pds);

  std::string pdcDescription = vtkF3DGenericImporter::GetDataObjectDescription(pdc);
  if (pdcDescription.empty())
  {
    std::cerr << "GetDataObjectDescription(pdc) returned empty string\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkTable> table;
  vtkNew<vtkTrivialProducer> tableProducer;
  tableProducer->SetOutput(table);

  vtkNew<vtkF3DGenericImporter> tableImporter;
  tableImporter->SetInternalReader(tableProducer);
  tableImporter->Update();

  if (tableImporter->GetNumberOfBlocks() != 0)
  {
    std::cerr << "Importer with vtkTable should have 0 blocks\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkGLTFReader> animReader;
  std::string animFilename = std::string(argv[1]) + "data/BoxAnimated.gltf";
  animReader->SetFileName(animFilename.c_str());
  animReader->UpdateInformation();
  animReader->EnableAnimation(0);

  vtkNew<vtkF3DGenericImporter> animImporter;
  animImporter->SetInternalReader(animReader);
  animImporter->Update();
  animImporter->EnableAnimation(0);

  animReader->SetFileName("/nonexistent/path/file.gltf");

  if (animImporter->UpdateAtTimeValue(0.5))
  {
    std::cerr << "UpdateAtTimeValue should fail with invalid reader\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
