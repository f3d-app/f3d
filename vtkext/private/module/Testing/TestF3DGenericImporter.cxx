#include <vtkConeSource.h>
#include <vtkDataAssembly.h>
#include <vtkDoubleArray.h>
#include <vtkGLTFReader.h>
#include <vtkInformation.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkNew.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTable.h>
#include <vtkTrivialProducer.h>
#include <vtkVersion.h>
#include <vtkXMLMultiBlockDataReader.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporter(int argc, char* argv[])
{
  // Create reusable test data
  vtkNew<vtkSphereSource> sphere;
  sphere->Update();
  vtkNew<vtkConeSource> cone;
  cone->Update();

  // Test basic importer state
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
  }

  // Test animation with GLTF reader
  {
    vtkNew<vtkGLTFReader> reader;
    std::string filename = std::string(argv[1]) + "data/BoxAnimated.gltf";
    reader->SetFileName(filename.c_str());
    reader->UpdateInformation();
    reader->EnableAnimation(0);

    vtkNew<vtkF3DGenericImporter> importer;
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
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240910)
  // Test UpdateAtTimeValue with invalid animation
  {
    vtkNew<vtkGLTFReader> reader;
    std::string filename = std::string(argv[1]) + "data/BoxAnimated_invalid_animation.gltf";
    reader->SetFileName(filename.c_str());
    reader->UpdateInformation();
    reader->EnableAnimation(0);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(reader);
    importer->Update();
    if (!importer->UpdateAtTimeValue(0.1))
    {
      std::cerr << "Unexpected UpdateAtTimeValue failure\n";
      return EXIT_FAILURE;
    }
  }
#endif

  // Test UpdateAtTimeValue failure path
  {
    vtkNew<vtkGLTFReader> reader;
    std::string filename = std::string(argv[1]) + "data/BoxAnimated.gltf";
    reader->SetFileName(filename.c_str());
    reader->UpdateInformation();
    reader->EnableAnimation(0);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(reader);
    importer->Update();
    importer->EnableAnimation(0);

    reader->SetFileName("/nonexistent/path/file.gltf");
    if (importer->UpdateAtTimeValue(0.5))
    {
      std::cerr << "UpdateAtTimeValue should fail with invalid reader\n";
      return EXIT_FAILURE;
    }
  }

  // Test GetDataObjectDescription
  {
    if (vtkF3DGenericImporter::GetDataObjectDescription(nullptr) != "")
    {
      std::cerr << "GetDataObjectDescription(nullptr) should return empty\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkPartitionedDataSet> pds;
    pds->SetNumberOfPartitions(1);
    pds->SetPartition(0, sphere->GetOutput());

    vtkNew<vtkPartitionedDataSetCollection> pdc;
    pdc->SetNumberOfPartitionedDataSets(1);
    pdc->SetPartitionedDataSet(0, pds);

    if (vtkF3DGenericImporter::GetDataObjectDescription(pdc).empty())
    {
      std::cerr << "GetDataObjectDescription(pdc) should not be empty\n";
      return EXIT_FAILURE;
    }
  }

  // Test failure path with unsupported data type (vtkTable)
  {
    vtkNew<vtkTable> table;
    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(table);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetNumberOfBlocks() != 0)
    {
      std::cerr << "Importer with vtkTable should have 0 blocks\n";
      return EXIT_FAILURE;
    }
  }

  // Test MultiBlock from file (covers null blocks, nested structures)
  {
    vtkNew<vtkXMLMultiBlockDataReader> reader;
    std::string filename = std::string(argv[1]) + "data/mb.vtm";
    reader->SetFileName(filename.c_str());

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(reader);
    importer->Update();

    // mb.vtm contains 3 dataset blocks, vtkTable is skipped
    if (importer->GetNumberOfBlocks() != 3)
    {
      std::cerr << "MB file: Expected 3 blocks, got " << importer->GetNumberOfBlocks() << "\n";
      return EXIT_FAILURE;
    }

    // Test out-of-bounds access
    if (importer->GetImportedPoints(-1) != nullptr || importer->GetImportedPoints(3) != nullptr ||
      importer->GetImportedImage(-1) != nullptr || importer->GetImportedImage(3) != nullptr ||
      !importer->GetBlockName(-1).empty() || !importer->GetBlockName(3).empty())
    {
      std::cerr << "Out-of-bounds access should return nullptr/empty\n";
      return EXIT_FAILURE;
    }
  }

  // Test MultiBlock with metadata names
  {
    vtkNew<vtkMultiBlockDataSet> mb;
    mb->SetNumberOfBlocks(2);
    mb->SetBlock(0, sphere->GetOutput());
    mb->SetBlock(1, cone->GetOutput());
    mb->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "MySphere");
    mb->GetMetaData(1u)->Set(vtkCompositeDataSet::NAME(), "MyCone");

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(mb);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetBlockName(0) != "MySphere" || importer->GetBlockName(1) != "MyCone")
    {
      std::cerr << "MB metadata names not preserved\n";
      return EXIT_FAILURE;
    }
  }

  // Test MultiBlock without metadata (Block_N fallback)
  {
    vtkNew<vtkMultiBlockDataSet> mb;
    mb->SetNumberOfBlocks(2);
    mb->SetBlock(0, sphere->GetOutput());
    mb->SetBlock(1, cone->GetOutput());

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(mb);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetBlockName(0) != "Block_0" || importer->GetBlockName(1) != "Block_1")
    {
      std::cerr << "MB without metadata: Expected 'Block_N', got '" << importer->GetBlockName(0)
                << "', '" << importer->GetBlockName(1) << "'\n";
      return EXIT_FAILURE;
    }
  }

  // Test nested MultiBlock with hierarchical names
  {
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

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(outerMB);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetBlockName(0) != "OuterBlock/InnerSphere" ||
      importer->GetBlockName(1) != "OuterBlock/InnerCone")
    {
      std::cerr << "Nested MB: Expected hierarchical names\n";
      return EXIT_FAILURE;
    }
  }

  // Test MultiPieceDataSet nested inside MultiBlock (generic composite fallback)
  {
    vtkNew<vtkMultiPieceDataSet> mp;
    mp->SetNumberOfPieces(2);
    mp->SetPiece(0, sphere->GetOutput());
    mp->SetPiece(1, cone->GetOutput());
    mp->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "SpherePiece");
    mp->GetMetaData(1u)->Set(vtkCompositeDataSet::NAME(), "ConePiece");

    vtkNew<vtkMultiBlockDataSet> mb;
    mb->SetNumberOfBlocks(1);
    mb->SetBlock(0, mp);
    mb->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "NestedMP");

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(mb);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetNumberOfBlocks() != 2)
    {
      std::cerr << "Nested MP: Expected 2 blocks\n";
      return EXIT_FAILURE;
    }

    std::string name0 = importer->GetBlockName(0);
    std::string name1 = importer->GetBlockName(1);
    if (name0.find("NestedMP") == std::string::npos ||
      name0.find("SpherePiece") == std::string::npos ||
      name1.find("NestedMP") == std::string::npos || name1.find("ConePiece") == std::string::npos)
    {
      std::cerr << "Nested MP: Expected hierarchical names with piece metadata\n";
      return EXIT_FAILURE;
    }
  }

  // Test PartitionedDataSet with metadata names
  {
    vtkNew<vtkPartitionedDataSet> pds;
    pds->SetNumberOfPartitions(2);
    pds->SetPartition(0, sphere->GetOutput());
    pds->SetPartition(1, cone->GetOutput());
    pds->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "SpherePartition");
    pds->GetMetaData(1u)->Set(vtkCompositeDataSet::NAME(), "ConePartition");

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(pds);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    std::string name0 = importer->GetBlockName(0);
    std::string name1 = importer->GetBlockName(1);
    if (name0.find("SpherePartition") == std::string::npos ||
      name1.find("ConePartition") == std::string::npos)
    {
      std::cerr << "PDS: Expected partition metadata names\n";
      return EXIT_FAILURE;
    }
  }

  // Test PartitionedDataSet without metadata (Partition_N fallback)
  {
    vtkNew<vtkPartitionedDataSet> pds;
    pds->SetNumberOfPartitions(2);
    pds->SetPartition(0, sphere->GetOutput());
    pds->SetPartition(1, cone->GetOutput());
    // No metadata

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(pds);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    std::string name0 = importer->GetBlockName(0);
    std::string name1 = importer->GetBlockName(1);
    if (name0.find("Partition_0") == std::string::npos ||
      name1.find("Partition_1") == std::string::npos)
    {
      std::cerr << "PDS without metadata: Expected 'Partition_N', got '" << name0 << "', '" << name1
                << "'\n";
      return EXIT_FAILURE;
    }
  }

  // Test PartitionedDataSet with null partition
  {
    vtkNew<vtkPartitionedDataSet> pds;
    pds->SetNumberOfPartitions(3);
    pds->SetPartition(0, sphere->GetOutput());
    pds->SetPartition(2, cone->GetOutput());

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(pds);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetNumberOfBlocks() != 2)
    {
      std::cerr << "PDS with null: Expected 2 blocks, got " << importer->GetNumberOfBlocks()
                << "\n";
      return EXIT_FAILURE;
    }
  }

  // Test PartitionedDataSetCollection with assembly
  {
    vtkNew<vtkPartitionedDataSet> pds0;
    pds0->SetNumberOfPartitions(1);
    pds0->SetPartition(0, sphere->GetOutput());

    vtkNew<vtkPartitionedDataSet> pds1;
    pds1->SetNumberOfPartitions(1);
    pds1->SetPartition(0, cone->GetOutput());

    vtkNew<vtkPartitionedDataSetCollection> pdc;
    pdc->SetNumberOfPartitionedDataSets(2);
    pdc->SetPartitionedDataSet(0, pds0);
    pdc->SetPartitionedDataSet(1, pds1);

    vtkNew<vtkDataAssembly> assembly;
    assembly->Initialize();
    int sphereNode = assembly->AddNode("SphereFromAssembly", assembly->GetRootNode());
    int coneNode = assembly->AddNode("ConeFromAssembly", assembly->GetRootNode());
    assembly->AddDataSetIndex(sphereNode, 0);
    assembly->AddDataSetIndex(coneNode, 1);
    pdc->SetDataAssembly(assembly);

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(pdc);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetBlockName(0) != "SphereFromAssembly" ||
      importer->GetBlockName(1) != "ConeFromAssembly")
    {
      std::cerr << "PDC with assembly: Expected assembly names\n";
      return EXIT_FAILURE;
    }
  }

  // Test PartitionedDataSetCollection with metadata (no assembly)
  {
    vtkNew<vtkPartitionedDataSet> pds0;
    pds0->SetNumberOfPartitions(1);
    pds0->SetPartition(0, sphere->GetOutput());

    vtkNew<vtkPartitionedDataSetCollection> pdc;
    pdc->SetNumberOfPartitionedDataSets(1);
    pdc->SetPartitionedDataSet(0, pds0);
    pdc->GetMetaData(0u)->Set(vtkCompositeDataSet::NAME(), "MetadataName");

    vtkNew<vtkTrivialProducer> producer;
    producer->SetOutput(pdc);

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(producer);
    importer->Update();

    if (importer->GetBlockName(0) != "MetadataName")
    {
      std::cerr << "PDC with metadata: Expected 'MetadataName', got '" << importer->GetBlockName(0)
                << "'\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
