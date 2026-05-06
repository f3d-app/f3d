#include <vtkActor.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// We need to include the importer header
#include "plugins/open3sdcm/module/vtkF3DOpen3SDCMImporter.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <dcm_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string filename = argv[1];
    
    vtkNew<vtkF3DOpen3SDCMImporter> importer;
    importer->SetFileName(filename.c_str());

    if (!importer->ImportBegin())
    {
        std::cerr << "Failed to import file" << std::endl;
        return EXIT_FAILURE;
    }

    // Create a dummy renderer
    vtkNew<vtkRenderWindow> renderWindow;
    vtkNew<vtkRenderer> renderer;
    renderWindow->AddRenderer(renderer);
    importer->SetRenderWindow(renderWindow);
    importer->Update();

    // Get actors
    vtkActorCollection* actors = importer->GetImportedActors();
    actors->InitTraversal();
    vtkActor* actor = actors->GetNextActor();

    if (!actor)
    {
        std::cerr << "No actors found" << std::endl;
        return EXIT_FAILURE;
    }

    vtkPolyData* polyData = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInputDataObject(0));
    if (!polyData)
    {
        std::cerr << "No polydata found" << std::endl;
        return EXIT_FAILURE;
    }

    vtkFloatArray* tcoords = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetTCoords());
    if (!tcoords)
    {
        std::cout << "NO_TEXTURE_COORDINATES" << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "DCM_UV_COORDS" << std::endl;
    std::cout << "Index, U, V" << std::endl;
    int count = 0;
    for (vtkIdType i = 0; i < tcoords->GetNumberOfTuples() && count < 20; i++)
    {
        float* uv = tcoords->GetTuple2(i);
        std::cout << i << ", " << uv[0] << ", " << uv[1] << std::endl;
        count++;
    }

    return EXIT_SUCCESS;
}
