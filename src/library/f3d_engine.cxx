#include "f3d_engine.h"

#include "f3d_config.h"
#include "f3d_init.h" // This includes is needed on mac so that the global variable is created
#include "f3d_interactor_impl.h"
#include "f3d_loader_impl.h"
#include "f3d_log.h"
#include "f3d_options.h"
#include "f3d_window_impl_noRender.h"
#include "f3d_window_impl_standard.h"

#include "F3DReaderFactory.h"

#include "vtkF3DConfigure.h"

#include <vtkVersion.h>

namespace f3d
{
class engine::F3DInternals
{
public:
  std::unique_ptr<options> Options;
  std::unique_ptr<window_impl> Window;
  std::unique_ptr<loader_impl> Loader;
  std::unique_ptr<interactor_impl> Interactor;
};

//----------------------------------------------------------------------------
engine::engine(const flags_t& flags)
  : Internals(new engine::F3DInternals())
{
  this->Internals->Options = std::make_unique<options>();

  if (flags & CREATE_WINDOW)
  {
    this->Internals->Window =
      std::make_unique<window_impl_standard>(*this->Internals->Options, flags & WINDOW_OFFSCREEN);
  }
  else
  {
    // Without the window flag, we still need to create a window noRender
    this->Internals->Window = std::make_unique<window_impl_noRender>(*this->Internals->Options);
  }

  this->Internals->Loader =
    std::make_unique<loader_impl>(*this->Internals->Options, *this->Internals->Window);

  if (flags & CREATE_INTERACTOR)
  {
    this->Internals->Interactor = std::make_unique<interactor_impl>(
      *this->Internals->Options, *this->Internals->Window, *this->Internals->Loader);
  }
}

//----------------------------------------------------------------------------
engine::~engine()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void engine::setOptions(const options& opt)
{
  *this->Internals->Options = opt;
}

//----------------------------------------------------------------------------
void engine::setOptions(options&& opt)
{
  *this->Internals->Options = std::move(opt);
}

//----------------------------------------------------------------------------
options& engine::getOptions()
{
  return *this->Internals->Options;
}

//----------------------------------------------------------------------------
window& engine::getWindow()
{
  if (!this->Internals->Window ||
    dynamic_cast<window_impl_noRender*>(this->Internals->Window.get()))
  {
    throw f3d::engine::exception("Cannot create window with this engine");
  }
  return *this->Internals->Window;
}

//----------------------------------------------------------------------------
loader& engine::getLoader()
{
  return *this->Internals->Loader;
}

//----------------------------------------------------------------------------
interactor& engine::getInteractor()
{
  if (!this->Internals->Interactor)
  {
    throw f3d::engine::exception("Cannot create interactor with this engine");
  }
  return *this->Internals->Interactor;
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> engine::getLibInfo()
{
  std::map<std::string, std::string> libInfo;
  libInfo["Version"] = f3d::LibVersion;
  libInfo["Build date"] = f3d::LibBuildDate;
  libInfo["Build system"] = f3d::LibBuildSystem;
  libInfo["Compiler"] = f3d::LibCompiler;

  std::string tmp;

#if F3D_MODULE_RAYTRACING
  tmp = "ON";
#else
  tmp = "OFF";
#endif
  libInfo["Raytracing module"] = tmp;

#if F3D_MODULE_EXODUS
  tmp = "ON";
#else
  tmp = "OFF";
#endif
  libInfo["Exodus module"] = tmp;

#if F3D_MODULE_OCCT
  tmp = F3D_OCCT_VERSION;
#if F3D_MODULE_OCCT_XCAF
  tmp += " (full support)";
#else
  tmp += " (no metadata)";
#endif
#else
  tmp = "OFF";
#endif
  libInfo["OpenCASCADE module"] = tmp;

#if F3D_MODULE_ASSIMP
  tmp = F3D_ASSIMP_VERSION;
#else
  tmp = "OFF";
#endif
  libInfo["Assimp module"] = tmp;

#if F3D_MODULE_ALEMBIC
  tmp = F3D_ALEMBIC_VERSION;
#else
  tmp = "OFF";
#endif
  libInfo["Alembic module"] = tmp;

  libInfo["VTK version"] = std::string(VTK_VERSION) + std::string(" (build ") +
    std::to_string(VTK_BUILD_VERSION) + std::string(")");

  libInfo["Copyright_previous"] = "Copyright (C) 2019-2021 Kitware SAS";
  libInfo["Copyright"] = "Copyright (C) 2021-2022 Michael Migliore, Mathieu Westphal";
  libInfo["License"] = "BSD-3-Clause";
  libInfo["Authors"] = "Michael Migliore, Mathieu Westphal and Joachim Pouderoux";

  return libInfo;
}

//----------------------------------------------------------------------------
std::vector<f3d::engine::readerInformation> engine::getReadersInfo()
{
  std::vector<readerInformation> readersInfo;
  const auto& readers = F3DReaderFactory::GetInstance()->GetReaders();
  for (const auto& reader : readers)
  {
    readerInformation info;
    info.name = reader->GetName();
    info.description = reader->GetLongDescription();
    info.extensions = reader->GetExtensions();
    info.mimetypes = reader->GetMimeTypes();
    readersInfo.push_back(info);
  }
  return readersInfo;
}
}
