#include <vtkConeSource.h>
#include <vtkDataAssembly.h>
#include <vtkDoubleArray.h>
#include <vtkGLTFReader.h>
#include <vtkInformation.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTable.h>
#include <vtkTestUtilities.h>
#include <vtkTrivialProducer.h>
#include <vtkUniformGrid.h>
#include <vtkUniformGridAMR.h>
#include <vtkVersion.h>
#include <vtkXMLMultiBlockDataReader.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>
#include <vector>

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

  // Test block names for vtkMultiBlockDataSet with metadata names
  {
    vtkNew<vtkSphereSource> sphere;
    sphere->Update();
    vtkNew<vtkConeSource> cone;
    cone->Update();

    vtkNew<vtkMultiBlockDataSet> mb;
    mb->SetNumberOfBlocks(2);
    mb->SetBlock(0, sphere->GetOutput());
    mb->SetBlock(1, cone->GetOutput());
    mb->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "MySphere");
    mb->GetMetaData(1u)->Set(vtkCompositeDataSet::NAME(), "MyCone");

    vtkNew<vtkTrivialProducer> mbProducer;
    mbProducer->SetOutput(mb);

    vtkNew<vtkF3DGenericImporter> mbNameImporter;
    mbNameImporter->SetInternalReader(mbProducer);
    mbNameImporter->Update();

    if (mbNameImporter->GetNumberOfBlocks() != 2)
    {
      std::cerr << "MultiBlock: Expected 2 blocks, got " << mbNameImporter->GetNumberOfBlocks()
                << "\n";
      return EXIT_FAILURE;
    }
    if (mbNameImporter->GetBlockName(0) != "MySphere")
    {
      std::cerr << "MultiBlock: Expected 'MySphere', got '" << mbNameImporter->GetBlockName(0)
                << "'\n";
      return EXIT_FAILURE;
    }
    if (mbNameImporter->GetBlockName(1) != "MyCone")
    {
      std::cerr << "MultiBlock: Expected 'MyCone', got '" << mbNameImporter->GetBlockName(1)
                << "'\n";
      return EXIT_FAILURE;
    }
  }

  // Test block names for vtkPartitionedDataSet
  {
    vtkNew<vtkSphereSource> sphere;
    sphere->Update();
    vtkNew<vtkConeSource> cone;
    cone->Update();

    vtkNew<vtkPartitionedDataSet> pdsTest;
    pdsTest->SetNumberOfPartitions(2);
    pdsTest->SetPartition(0, sphere->GetOutput());
    pdsTest->SetPartition(1, cone->GetOutput());
    pdsTest->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "SpherePartition");
    pdsTest->GetMetaData(1u)->Set(vtkCompositeDataSet::NAME(), "ConePartition");

    vtkNew<vtkTrivialProducer> pdsProducer;
    pdsProducer->SetOutput(pdsTest);

    vtkNew<vtkF3DGenericImporter> pdsImporter;
    pdsImporter->SetInternalReader(pdsProducer);
    pdsImporter->Update();

    if (pdsImporter->GetNumberOfBlocks() != 2)
    {
      std::cerr << "PDS: Expected 2 blocks, got " << pdsImporter->GetNumberOfBlocks() << "\n";
      return EXIT_FAILURE;
    }
    // PDS with no parent name should use default + partition name
    std::string name0 = pdsImporter->GetBlockName(0);
    std::string name1 = pdsImporter->GetBlockName(1);
    if (name0.find("SpherePartition") == std::string::npos)
    {
      std::cerr << "PDS: Expected name containing 'SpherePartition', got '" << name0 << "'\n";
      return EXIT_FAILURE;
    }
    if (name1.find("ConePartition") == std::string::npos)
    {
      std::cerr << "PDS: Expected name containing 'ConePartition', got '" << name1 << "'\n";
      return EXIT_FAILURE;
    }
  }

  // Test block names for vtkPartitionedDataSetCollection with assembly
  {
    vtkNew<vtkSphereSource> sphere;
    sphere->Update();
    vtkNew<vtkConeSource> cone;
    cone->Update();

    vtkNew<vtkPartitionedDataSet> pds0;
    pds0->SetNumberOfPartitions(1);
    pds0->SetPartition(0, sphere->GetOutput());

    vtkNew<vtkPartitionedDataSet> pds1;
    pds1->SetNumberOfPartitions(1);
    pds1->SetPartition(0, cone->GetOutput());

    vtkNew<vtkPartitionedDataSetCollection> pdcTest;
    pdcTest->SetNumberOfPartitionedDataSets(2);
    pdcTest->SetPartitionedDataSet(0, pds0);
    pdcTest->SetPartitionedDataSet(1, pds1);

    vtkNew<vtkDataAssembly> assembly;
    assembly->Initialize();
    int sphereNode = assembly->AddNode("SphereFromAssembly", assembly->GetRootNode());
    int coneNode = assembly->AddNode("ConeFromAssembly", assembly->GetRootNode());
    assembly->AddDataSetIndex(sphereNode, 0);
    assembly->AddDataSetIndex(coneNode, 1);
    pdcTest->SetDataAssembly(assembly);

    vtkNew<vtkTrivialProducer> pdcProducer;
    pdcProducer->SetOutput(pdcTest);

    vtkNew<vtkF3DGenericImporter> pdcImporter;
    pdcImporter->SetInternalReader(pdcProducer);
    pdcImporter->Update();

    if (pdcImporter->GetNumberOfBlocks() != 2)
    {
      std::cerr << "PDC: Expected 2 blocks, got " << pdcImporter->GetNumberOfBlocks() << "\n";
      return EXIT_FAILURE;
    }
    if (pdcImporter->GetBlockName(0) != "SphereFromAssembly")
    {
      std::cerr << "PDC: Expected 'SphereFromAssembly', got '" << pdcImporter->GetBlockName(0)
                << "'\n";
      return EXIT_FAILURE;
    }
    if (pdcImporter->GetBlockName(1) != "ConeFromAssembly")
    {
      std::cerr << "PDC: Expected 'ConeFromAssembly', got '" << pdcImporter->GetBlockName(1)
                << "'\n";
      return EXIT_FAILURE;
    }
  }

  // Test nested vtkMultiBlockDataSet with hierarchical names
  {
    vtkNew<vtkSphereSource> sphere;
    sphere->Update();
    vtkNew<vtkConeSource> cone;
    cone->Update();

    vtkNew<vtkMultiBlockDataSet> innerMB;
    innerMB->SetNumberOfBlocks(2);
    innerMB->SetBlock(0, sphere->GetOutput());
    innerMB->SetBlock(1, cone->GetOutput());
    innerMB->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "InnerSphere");
    innerMB->GetMetaData(1u)->Set(vtkCompositeDataSet::NAME(), "InnerCone");

    vtkNew<vtkMultiBlockDataSet> outerMB;
    outerMB->SetNumberOfBlocks(1);
    outerMB->SetBlock(0, innerMB);
    outerMB->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "OuterBlock");

    vtkNew<vtkTrivialProducer> nestedProducer;
    nestedProducer->SetOutput(outerMB);

    vtkNew<vtkF3DGenericImporter> nestedImporter;
    nestedImporter->SetInternalReader(nestedProducer);
    nestedImporter->Update();

    if (nestedImporter->GetNumberOfBlocks() != 2)
    {
      std::cerr << "Nested MB: Expected 2 blocks, got " << nestedImporter->GetNumberOfBlocks()
                << "\n";
      return EXIT_FAILURE;
    }
    // Names should be hierarchical: OuterBlock/InnerSphere, OuterBlock/InnerCone
    std::string name0 = nestedImporter->GetBlockName(0);
    std::string name1 = nestedImporter->GetBlockName(1);
    if (name0 != "OuterBlock/InnerSphere")
    {
      std::cerr << "Nested MB: Expected 'OuterBlock/InnerSphere', got '" << name0 << "'\n";
      return EXIT_FAILURE;
    }
    if (name1 != "OuterBlock/InnerCone")
    {
      std::cerr << "Nested MB: Expected 'OuterBlock/InnerCone', got '" << name1 << "'\n";
      return EXIT_FAILURE;
    }
  }

  // Test generic composite
  {
    vtkNew<vtkUniformGridAMR> amr;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 0)
    std::vector<unsigned int> blocksPerLevel = { 1 };
    amr->Initialize(blocksPerLevel);
#else
    int blocksPerLevel[1] = { 1 };
    amr->Initialize(1, blocksPerLevel);
#endif

    vtkNew<vtkUniformGrid> grid;
    grid->SetDimensions(3, 3, 3);
    grid->SetOrigin(0, 0, 0);
    grid->SetSpacing(1.0, 1.0, 1.0);
    amr->SetDataSet(0, 0, grid);

    vtkNew<vtkTrivialProducer> amrProducer;
    amrProducer->SetOutput(amr);

    vtkNew<vtkF3DGenericImporter> amrImporter;
    amrImporter->SetInternalReader(amrProducer);
    amrImporter->Update();

    if (amrImporter->GetNumberOfBlocks() < 1)
    {
      std::cerr << "AMR: Expected at least 1 block, got " << amrImporter->GetNumberOfBlocks()
                << "\n";
      return EXIT_FAILURE;
    }

    const vtkPolyData* points = amrImporter->GetImportedPoints(0);
    if (!points)
    {
      std::cerr << "AMR: GetImportedPoints(0) returned nullptr\n";
      return EXIT_FAILURE;
    }

    std::string blockName = amrImporter->GetBlockName(0);
    if (blockName.empty())
    {
      std::cerr << "AMR: GetBlockName(0) returned empty string\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
