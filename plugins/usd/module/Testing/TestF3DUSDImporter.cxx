#include "vtkF3DUSDImporter.h"

#include <vtkDataAssembly.h>
#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkVersion.h>

#include <iostream>

int TestF3DUSDImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.usd";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->DisableAnimation(0);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
  if (importer->GetAnimationSupportLevel() != vtkImporter::AnimationSupportLevel::UNIQUE)
  {
    return EXIT_FAILURE;
  }
#endif

  importer->Update();
  importer->Print(std::cout);

  if (!importer->GetRenderer())
  {
    std::cerr << "No renderer after import" << std::endl;
    return EXIT_FAILURE;
  }

  // Test scene hierarchy with primitives.usda which has a known hierarchy structure
  std::string primitivesFile = std::string(argv[1]) + "data/primitives.usda";
  vtkNew<vtkF3DUSDImporter> primImporter;
  primImporter->SetFileName(primitivesFile.c_str());
  primImporter->Update();

  vtkDataAssembly* hierarchy = primImporter->GetSceneHierarchy();
  if (!hierarchy)
  {
    std::cerr << "Scene hierarchy is null" << std::endl;
    return EXIT_FAILURE;
  }

  // Print the hierarchy for debugging
  std::cout << "Scene hierarchy:" << std::endl;
  hierarchy->Print(std::cout);

  // Verify the hierarchy has nodes (root + Primitives + 5 primitives = at least 7 nodes)
  std::vector<int> allNodes = hierarchy->SelectNodes({ "//*" });
  int numNodes = static_cast<int>(allNodes.size());
  std::cout << "Number of nodes: " << numNodes << std::endl;

  if (numNodes < 6)
  {
    std::cerr << "Expected at least 6 nodes in hierarchy, got " << numNodes << std::endl;
    return EXIT_FAILURE;
  }

  // Verify we have actors with flat_actor_id attributes
  std::vector<int> actorNodes = hierarchy->SelectNodes({ "//*[@flat_actor_id]" });
  std::cout << "Number of actor nodes: " << actorNodes.size() << std::endl;

  if (actorNodes.size() != 5)
  {
    std::cerr << "Expected 5 actor nodes, got " << actorNodes.size() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
