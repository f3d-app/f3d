#include <vtkGLTFReader.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTestUtilities.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "vtkF3DGenericImporter.h"
#include "vtkF3DInteractorStyle.h"
#include "vtkF3DRenderer.h"

#include <iostream>

int TestF3DRendererWithColoring(int argc, char* argv[])
{
  // Test invalid data
  {
    vtkNew<vtkF3DRenderer> renderer;
    vtkNew<vtkF3DMetaImporter> importer;
    vtkNew<vtkRenderWindow> window;

    window->AddRenderer(renderer);
    importer->SetRenderWindow(window);
    renderer->SetImporter(importer);

    // Check invalid bounding box code path
    renderer->ShowGrid(true);
    renderer->UpdateActors();

    vtkNew<vtkXMLUnstructuredGridReader> readerVTU;
    const std::string filename = std::format("{}data/bluntfin_t.vtu", argv[1]);
    readerVTU->SetFileName(filename.c_str());
    vtkNew<vtkF3DGenericImporter> importerVTU;
    importerVTU->SetInternalReader(readerVTU);

    importer->AddImporter({ "foo", importerVTU });
    importer->Update();

    // Check invalid array code path
    renderer->SetEnableColoring(true);
    renderer->SetArrayNameForColoring("Invalid");
    renderer->SetUseVolume(false);
    renderer->UpdateActors();
    renderer->CycleComponentForColoring();
    renderer->SetUseVolume(true);
    renderer->UpdateActors();

    if (renderer->GetArrayNameForColoring() != "Invalid" || renderer->GetComponentForColoring() != -1)
    {
      std::cerr << "Unexpected coloring information with invalid array\n";
      return EXIT_FAILURE;
    }

    // Check invalid component code path
    renderer->SetArrayNameForColoring("Momentum");
    renderer->SetComponentForColoring(5);
    renderer->UpdateActors();
    renderer->SetUseVolume(true);
    renderer->UpdateActors();

    if (renderer->GetArrayNameForColoring() != "Momentum" || renderer->GetComponentForColoring() != 5)
    {
      std::cerr << "Unexpected coloring information with invalid component\n";
      return EXIT_FAILURE;
    }

    renderer->CycleComponentForColoring();
    if (renderer->GetArrayNameForColoring() != "Momentum" || renderer->GetComponentForColoring() != 1)
    {
      std::cerr << "Unexpected coloring information after cycling component\n";
      return EXIT_FAILURE;
    }

    // Check invalid colormap code path
    renderer->SetColormap({ 0, 0, 0 });
    renderer->UpdateActors();

    // Smoke test for deprecated HDRI collapse codepath
    // F3D_DEPRECATED
    renderer->SetHDRIFile("path/not/valid/../../to/file.ext");

    // Check SetInteractionStyle without interactor (early return)
    renderer->SetInteractionStyle("default");

    // Check SetInteractionStyle with invalid style
    vtkNew<vtkRenderWindowInteractor> interactor;
    window->SetInteractor(interactor);
    vtkNew<vtkF3DInteractorStyle> style;
    interactor->SetInteractorStyle(style);
    renderer->SetInteractionStyle("invalid");
  }

  // Test depth scalar coloring
  {
    vtkNew<vtkRenderWindow> window;
    vtkNew<vtkF3DRenderer> renderer;
    vtkNew<vtkF3DMetaImporter> importer;

    window->AddRenderer(renderer);
    importer->SetRenderWindow(window);
    renderer->SetImporter(importer);

    vtkNew<vtkGLTFReader> gltfReader;
    const std::string filename = std::format("{}data/WaterBottle.glb", argv[1]);
    gltfReader->SetFileName(filename.c_str());
    vtkNew<vtkF3DGenericImporter> gltfImporter;
    gltfImporter->SetInternalReader(gltfReader);

    importer->AddImporter({ "foo", gltfImporter });
    importer->Update();

    renderer->SetDisplayDepth(true);
    renderer->SetEnableColoring(true);
    renderer->SetArrayNameForColoring("NORMAL");
    renderer->SetUseVolume(false);
    renderer->ShowScalarBar(true);
    renderer->UpdateActors();

    const F3DColoringInfoHandler& coloringHandler = importer->GetColoringInfoHandler();
    const auto coloringInfo = coloringHandler.GetCurrentColoringInfo();

    if (!coloringInfo.has_value())
    {
      std::cerr << "Unexpected coloring information\n";
      return EXIT_FAILURE;
    }

    if (coloringInfo.value().Name != "NORMAL")
    {
      std::cerr << "Unexpected coloring information name: " << coloringInfo.value().Name << "\n";
      return EXIT_FAILURE;
    }

    // Component should not matter when display depth is set
    if (renderer->ComponentToString(-1) != "Depth")
    {
      std::cerr << "Unexpected coloring component\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
