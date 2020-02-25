#include "F3DLoader.h"

#include "vtkF3DGenericImporter.h"

#include <vtk3DSImporter.h>
#include <vtkActor2DCollection.h>
#include <vtkCallbackCommand.h>
#include <vtkGLTFImporter.h>
#include <vtkNew.h>
#include <vtkOBJImporter.h>
#include <vtkVRMLImporter.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include "F3DLog.h"
#include "F3DOptions.h"

#include <algorithm>

//----------------------------------------------------------------------------
F3DLoader& F3DLoader::GetInstance()
{
  static F3DLoader instance;
  return instance;
}

//----------------------------------------------------------------------------
F3DLoader::F3DLoader() = default;

//----------------------------------------------------------------------------
F3DLoader::~F3DLoader() = default;

//----------------------------------------------------------------------------
void F3DLoader::AddFiles(const std::vector<std::string>& files)
{
  for (auto& file : files)
  {
    this->AddFile(file);
  }
}

//----------------------------------------------------------------------------
void F3DLoader::AddFile(const std::string& path, bool recursive)
{
  if (path.empty() || !vtksys::SystemTools::FileExists(path))
  {
    return;
  }

  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);

  if (vtksys::SystemTools::FileIsDirectory(fullPath) && recursive)
  {
    vtksys::Directory dir;
    dir.Load(fullPath);

    for (unsigned long i = 0; i < dir.GetNumberOfFiles(); i++)
    {
      std::string currentFile = dir.GetFile(i);
      if (currentFile != "." && currentFile != "..")
      {
        this->AddFile(vtksys::SystemTools::JoinPath({ "", fullPath, currentFile }), true);
      }
    }
  }
  else
  {
    auto it = std::find(this->FilesList.begin(), this->FilesList.end(), fullPath);

    if (it == this->FilesList.end())
    {
      this->FilesList.push_back(fullPath);
    }
  }
}

//----------------------------------------------------------------------------
void F3DLoader::LoadPrevious(vtkF3DRenderer* ren)
{
  if (this->CurrentFileIndex > 0)
  {
    this->CurrentFileIndex--;
  }
  else
  {
    this->CurrentFileIndex = this->FilesList.size() - 1;
  }
  this->LoadCurrentIndex(ren);
}

//----------------------------------------------------------------------------
void F3DLoader::LoadNext(vtkF3DRenderer* ren)
{
  if (this->CurrentFileIndex < this->FilesList.size() - 1)
  {
    this->CurrentFileIndex++;
  }
  else
  {
    this->CurrentFileIndex = 0;
  }
  this->LoadCurrentIndex(ren);
}

//----------------------------------------------------------------------------
void F3DLoader::LoadCurrentIndex(vtkF3DRenderer* ren)
{
  if (this->CurrentFileIndex < 0 || this->CurrentFileIndex >= this->FilesList.size())
  {
    F3DLog::Print(F3DLog::Severity::Error, "Cannot load file index ", this->CurrentFileIndex);
    return;
  }

  std::string filePath = this->FilesList[this->CurrentFileIndex];
  std::string fileInfo = "(" + std::to_string(this->CurrentFileIndex + 1) + "/" +
    std::to_string(this->FilesList.size()) + ") " + vtksys::SystemTools::GetFilenameName(filePath);

  F3DOptions opts = F3DOptionsParser::GetInstance().GetOptionsFromFile(filePath);
  vtkSmartPointer<vtkImporter> importer = this->GetImporter(opts, filePath);
  if (!importer)
  {
    fileInfo += " [UNSUPPORTED]";
    ren->Initialize(opts, fileInfo);
    return;
  }

  ren->Initialize(opts, fileInfo);

  importer->SetRenderWindow(ren->GetRenderWindow());

  vtkNew<vtkProgressBarRepresentation> progressRep;
  vtkNew<vtkProgressBarWidget> progressWidget;

  if (opts.Progress)
  {
    vtkNew<vtkCallbackCommand> progressCallback;
    progressCallback->SetClientData(progressWidget);
    progressCallback->SetCallback([](vtkObject*, unsigned long, void* clientData, void* callData) {
      vtkProgressBarWidget* widget = static_cast<vtkProgressBarWidget*>(clientData);
      vtkProgressBarRepresentation* rep =
        vtkProgressBarRepresentation::SafeDownCast(widget->GetRepresentation());
      rep->SetProgressRate(*static_cast<double*>(callData));
      widget->GetInteractor()->GetRenderWindow()->Render();
    });
    importer->AddObserver(vtkCommand::ProgressEvent, progressCallback);

    progressWidget->SetInteractor(ren->GetRenderWindow()->GetInteractor());
    progressWidget->SetRepresentation(progressRep);

    progressRep->SetProgressRate(0.0);
    progressRep->SetPosition(0.25, 0.45);
    progressRep->SetProgressBarColor(1, 1, 1);
    progressRep->SetBackgroundColor(1, 1, 1);
    progressRep->DrawBackgroundOff();

    progressWidget->On();
  }

  importer->Update();

  progressWidget->Off();

  // Store scalar bar actor added by the generic importer
  vtkActor2DCollection* actors = ren->GetActors2D();
  actors->InitTraversal();
  vtkActor2D* currentActor;
  while ((currentActor = actors->GetNextActor2D()) != nullptr)
  {
    if (currentActor->IsA("vtkScalarBarActor"))
    {
      ren->SetScalarBarActor(currentActor);
      break;
    }
  }

  // Actors are loaded, use the bounds to reset camera and set-up SSAO
  ren->SetupRenderPasses();
  ren->ResetCamera();

  ren->ShowOptions();
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkImporter> F3DLoader::GetImporter(
  const F3DOptions& options, const std::string& file)
{
  if (!vtksys::SystemTools::FileExists(file))
  {
    cerr << "Specified input file '" << file << "' does not exist!" << endl;
    exit(EXIT_FAILURE);
  }

  if (!options.GeometryOnly)
  {
    std::string ext = vtksys::SystemTools::GetFilenameLastExtension(file);
    ext = vtksys::SystemTools::LowerCase(ext);

    if (ext == ".3ds")
    {
      vtkNew<vtk3DSImporter> importer;
      importer->SetFileName(file.c_str());
      importer->ComputeNormalsOn();
      return importer;
    }

    if (ext == ".obj")
    {
      vtkNew<vtkOBJImporter> importer;
      importer->SetFileName(file.c_str());

      std::string path = vtksys::SystemTools::GetFilenamePath(file);
      importer->SetTexturePath(path.c_str());

      // This logic is partially implemented in the OBJ importer itself
      // This complete version should be backported.
      std::string mtlFile = file + ".mtl";
      if (vtksys::SystemTools::FileExists(mtlFile))
      {
        importer->SetFileNameMTL(mtlFile.c_str());
      }
      else
      {
        mtlFile = path + "/" + vtksys::SystemTools::GetFilenameWithoutLastExtension(file) + ".mtl";
        if (vtksys::SystemTools::FileExists(mtlFile))
        {
          importer->SetFileNameMTL(mtlFile.c_str());
        }
      }

      return importer;
    }

    if (ext == ".wrl")
    {
      vtkNew<vtkVRMLImporter> importer;
      importer->SetFileName(file.c_str());
      return importer;
    }

    if (ext == ".gltf" || ext == ".glb")
    {
      vtkNew<vtkGLTFImporter> importer;
      importer->SetFileName(file.c_str());
      return importer;
    }
  }

  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetFileName(file.c_str());
  importer->SetOptions(options);

  if (!importer->CanReadFile())
  {
    return nullptr;
  }

  return importer;
}
