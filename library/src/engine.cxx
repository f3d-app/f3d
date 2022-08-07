#include "engine.h"

#include "config.h"
#include "init.h" // This includes is needed on mac so that the global variable is created
#include "interactor_impl.h"
#include "loader_impl.h"
#include "log.h"
#include "options.h"
#include "window_impl.h"

#include "F3DReaderFactory.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DNoRenderWindow.h"

#include <vtkVersion.h>

namespace f3d
{
class engine::internals
{
public:
  std::unique_ptr<options> Options;
  std::unique_ptr<detail::window_impl> Window;
  std::unique_ptr<detail::loader_impl> Loader;
  std::unique_ptr<detail::interactor_impl> Interactor;
};

//----------------------------------------------------------------------------
engine::engine(window::Type windowType)
  : Internals(new engine::internals)
{
  this->Internals->Options = std::make_unique<options>();

  this->Internals->Window =
    std::make_unique<detail::window_impl>(*this->Internals->Options, windowType);

  this->Internals->Loader =
    std::make_unique<detail::loader_impl>(*this->Internals->Options, *this->Internals->Window);

  // Do not create an interactor for NONE or EXTERNAL
  if (windowType != window::Type::NONE && windowType != window::Type::EXTERNAL)
  {
    this->Internals->Interactor = std::make_unique<detail::interactor_impl>(
      *this->Internals->Options, *this->Internals->Window, *this->Internals->Loader);
  }
}

//----------------------------------------------------------------------------
engine::~engine()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
engine& engine::setOptions(const options& opt)
{
  *this->Internals->Options = opt;
  return *this;
}

//----------------------------------------------------------------------------
engine& engine::setOptions(options&& opt)
{
  *this->Internals->Options = std::move(opt);
  return *this;
}

//----------------------------------------------------------------------------
options& engine::getOptions()
{
  return *this->Internals->Options;
}

//----------------------------------------------------------------------------
window& engine::getWindow()
{
  if (this->Internals->Window->getType() == window::Type::NONE)
  {
    throw engine::exception("No window with this engine");
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
    throw engine::exception("No interactor with this engine");
  }
  return *this->Internals->Interactor;
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> engine::getLibInfo()
{
  std::map<std::string, std::string> libInfo;
  libInfo["Version"] = detail::LibVersion;
  libInfo["Build date"] = detail::LibBuildDate;
  libInfo["Build system"] = detail::LibBuildSystem;
  libInfo["Compiler"] = detail::LibCompiler;

  std::string tmp;

#if F3D_MODULE_RAYTRACING
  tmp = "ON";
#else
  tmp = "OFF";
#endif
  libInfo["Raytracing module"] = tmp;

#if F3D_MODULE_EXTERNAL_RENDERING
  tmp = "ON";
#else
  tmp = "OFF";
#endif
  libInfo["External rendering module"] = tmp;

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
std::vector<engine::readerInformation> engine::getReadersInfo()
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
