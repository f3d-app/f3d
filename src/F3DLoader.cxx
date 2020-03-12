#include "F3DLoader.h"

#include "F3DLog.h"
#include "F3DNSDelegate.h"
#include "F3DOffscreenRender.h"
#include "F3DOptions.h"
#include "vtkF3DGenericImporter.h"
#include "vtkStringArray.h"

#include <vtk3DSImporter.h>
#include <vtkCallbackCommand.h>
#include <vtkGLTFImporter.h>
#include <vtkNew.h>
#include <vtkOBJImporter.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkScalarBarActor.h>
#include <vtkVRMLImporter.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <algorithm>

//----------------------------------------------------------------------------
F3DLoader::F3DLoader() = default;

//----------------------------------------------------------------------------
F3DLoader::~F3DLoader() = default;

//----------------------------------------------------------------------------
int F3DLoader::Start(int argc, char** argv)
{
  std::vector<std::string> files;

  this->Parser.Initialize(argc, argv);
  F3DOptions options = this->Parser.GetOptionsFromCommandLine(files);

  this->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();

  vtkNew<vtkRenderWindow> renWin;
  vtkNew<vtkRenderWindowInteractor> interactor;
  if (!options.NoRender)
  {
    renWin->SetSize(options.WindowSize[0], options.WindowSize[1]);
    renWin->SetWindowName(f3d::AppTitle.c_str());
    renWin->SetFullScreen(options.FullScreen);

    // the renderer must be added to the render window after OpenGL context initialization
    renWin->AddRenderer(this->Renderer);

    vtkNew<vtkF3DInteractorStyle> style;

    // Setup the observers for the interactor style events
    vtkNew<vtkCallbackCommand> newFilesCallback;
    newFilesCallback->SetClientData(this);
    newFilesCallback->SetCallback([](vtkObject*, unsigned long, void* clientData, void* callData) {
      F3DLoader* loader = static_cast<F3DLoader*>(clientData);
      loader->CurrentFileIndex = loader->FilesList.size();
      vtkStringArray* files = static_cast<vtkStringArray*>(callData);
      for (int i = 0; i < files->GetNumberOfTuples(); i++)
      {
      loader->AddFile(files->GetValue(i));
      }
      loader->LoadFile();
      });
    style->AddObserver(F3DLoader::NewFilesEvent, newFilesCallback);

    vtkNew<vtkCallbackCommand> loadFileCallback;
    loadFileCallback->SetClientData(this);
    loadFileCallback->SetCallback([](vtkObject*, unsigned long, void* clientData, void* callData) {
      F3DLoader* loader = static_cast<F3DLoader*>(clientData);
      int* load = static_cast<int*>(callData);
      loader->LoadFile(*load);
      });
    style->AddObserver(F3DLoader::LoadFileEvent, loadFileCallback);

    interactor->SetRenderWindow(renWin);
    interactor->SetInteractorStyle(style);
    interactor->Initialize();
    this->Renderer->Initialize(options, "");
  }

#if __APPLE__
  F3DNSDelegate::InitializeDelegate(this);
#endif

  if (files.size() > 0)
  {
    this->AddFiles(files);
    this->LoadFile();
  }

  int retVal = EXIT_SUCCESS;
  if (!options.NoRender)
  {
    if (!options.Output.empty())
    {
      retVal = F3DOffscreenRender::RenderOffScreen(renWin, options.Output);
    }
    else if (!options.Reference.empty())
    {
      retVal = F3DOffscreenRender::RenderTesting(renWin, options.Reference, options.RefThreshold);
    }
    else
    {
      renWin->Render();
      interactor->Start();
    }

    // for some reason, the widget should be disable before destruction
    this->Renderer->ShowAxis(false);
  }

  return retVal;
}

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
void F3DLoader::LoadFile(int load)
{
  // Compute the correct file index
  this->CurrentFileIndex = (this->CurrentFileIndex + load) % this->FilesList.size();

  if (this->CurrentFileIndex >= this->FilesList.size())
  {
    F3DLog::Print(F3DLog::Severity::Error, "Cannot load file index ", this->CurrentFileIndex);
    return;
  }

  std::string filePath = this->FilesList[this->CurrentFileIndex];
  std::string fileInfo = "(" + std::to_string(this->CurrentFileIndex + 1) + "/" +
    std::to_string(this->FilesList.size()) + ") " + vtksys::SystemTools::GetFilenameName(filePath);

  F3DOptions opts = this->Parser.GetOptionsFromFile(filePath);
  vtkSmartPointer<vtkImporter> importer = this->GetImporter(opts, filePath);

  vtkNew<vtkProgressBarWidget> progressWidget;
  if (!opts.NoRender)
  {
    if (!importer)
    {
      fileInfo += " [UNSUPPORTED]";
      this->Renderer->Initialize(opts, fileInfo);
      return;
    }

    this->Renderer->Initialize(opts, fileInfo);

    importer->SetRenderWindow(this->Renderer->GetRenderWindow());

    vtkNew<vtkProgressBarRepresentation> progressRep;

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

      progressWidget->SetInteractor(this->Renderer->GetRenderWindow()->GetInteractor());
      progressWidget->SetRepresentation(progressRep);

      progressRep->SetProgressRate(0.0);
      progressRep->SetPosition(0.25, 0.45);
      progressRep->SetProgressBarColor(1, 1, 1);
      progressRep->SetBackgroundColor(1, 1, 1);
      progressRep->DrawBackgroundOff();

      progressWidget->On();
    }
  }

  importer->Update();

  // Display description
  if (opts.Verbose || opts.NoRender)
  {
    F3DLog::Print(F3DLog::Severity::Info, importer->GetOutputsDescription());
  }

  if (!opts.NoRender)
  {
    progressWidget->Off();

    // Recover generic importer specific actors and mappers to set on the renderer
    vtkF3DGenericImporter* genericImporter = vtkF3DGenericImporter::SafeDownCast(importer);
    if (genericImporter)
    {
      this->Renderer->SetScalarBarActor(genericImporter->GetScalarBarActor());
      this->Renderer->SetGeometryActor(genericImporter->GetGeometryActor());
      this->Renderer->SetPolyDataMapper(genericImporter->GetPolyDataMapper());
      this->Renderer->SetPointGaussianMapper(genericImporter->GetPointGaussianMapper());
    }

    // Actors are loaded, use the bounds to reset camera and set-up SSAO
    this->Renderer->SetupRenderPasses();
    this->Renderer->ResetCamera();

    this->Renderer->ShowOptions();
  }
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkImporter> F3DLoader::GetImporter(
  const F3DOptions& options, const std::string& file)
{
  if (!vtksys::SystemTools::FileExists(file))
  {
    F3DLog::Print(F3DLog::Severity::Error, "Specified input file '", file, "' does not exist!");
    return nullptr;
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
