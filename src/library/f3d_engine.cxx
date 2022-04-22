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

#include <vtkVersion.h>

const f3d::engine::flags_t f3d::engine::FLAGS_NONE = f3d::engine::flags_t(0x0000);
const f3d::engine::flags_t f3d::engine::CREATE_WINDOW = f3d::engine::flags_t(0x0001);
const f3d::engine::flags_t f3d::engine::CREATE_INTERACTOR = f3d::engine::flags_t(0x0002);
const f3d::engine::flags_t f3d::engine::WINDOW_OFFSCREEN = f3d::engine::flags_t(0x0004);

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

  if ((flags & CREATE_WINDOW).any())
  {
    this->Internals->Window = std::make_unique<window_impl_standard>(
      *this->Internals->Options, (flags & WINDOW_OFFSCREEN).any());
  }
  else
  {
    // Without the window flag, we still need to create a window noRender
    this->Internals->Window = std::make_unique<window_impl_noRender>(*this->Internals->Options);
  }

  this->Internals->Loader =
    std::make_unique<loader_impl>(*this->Internals->Options, *this->Internals->Window);

  if ((flags & CREATE_INTERACTOR).any())
  {
    this->Internals->Interactor = std::make_unique<interactor_impl>(
      *this->Internals->Options, *this->Internals->Window, *this->Internals->Loader);
  }
}

//----------------------------------------------------------------------------
engine::~engine() = default;

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
    throw window_exception();
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
    throw interactor_exception();
  }
  return *this->Internals->Interactor;
}

//----------------------------------------------------------------------------
void engine::printVersion()
{
  // TODO engine should help with crafting the version string but should not be responsible for the
  // printing itself, to improve
  std::string version = f3d::AppName + " " + f3d::AppVersion + "\n\n";

  version += f3d::AppTitle;
  version += "\nVersion: ";
  version += f3d::AppVersion;
  version += "\nBuild date: ";
  version += f3d::AppBuildDate;
  version += "\nSystem: ";
  version += f3d::AppBuildSystem;
  version += "\nCompiler: ";
  version += f3d::AppCompiler;
  version += "\nRaytracing module: ";
#if F3D_MODULE_RAYTRACING
  version += "ON";
#else
  version += "OFF";
#endif
  version += "\nExodus module: ";
#if F3D_MODULE_EXODUS
  version += "ON";
#else
  version += "OFF";
#endif
  version += "\nOpenCASCADE module: ";
#if F3D_MODULE_OCCT
  version += F3D_OCCT_VERSION;
#if F3D_MODULE_OCCT_XCAF
  version += " (full support)";
#else
  version += " (no metadata)";
#endif
#else
  version += "OFF";
#endif
  version += "\nAssimp module: ";
#if F3D_MODULE_ASSIMP
  version += F3D_ASSIMP_VERSION;
#else
  version += "OFF";
#endif
  version += "\nVTK version: ";
  version += std::string(VTK_VERSION) + std::string(" (build ") +
    std::to_string(VTK_BUILD_VERSION) + std::string(")");

  version += "\n\nCopyright (C) 2019-2021 Kitware SAS.";
  version += "\nCopyright (C) 2021-2022 Michael Migliore, Mathieu Westphal.";
  version += "\nLicense BSD-3-Clause.";
  version += "\nWritten by Michael Migliore, Mathieu Westphal and Joachim Pouderoux.";

  f3d::log::setUseColoring(false);
  f3d::log::info(version);
  f3d::log::setUseColoring(true);
  f3d::log::waitForUser();
}

void engine::printReadersList()
{
  // TODO engin should help with crafting the reader list string but should not be responsible for
  // the printing itself, to improve
  size_t nameColSize = 0;
  size_t extsColSize = 0;
  size_t mimeColSize = 0;
  size_t descColSize = 0;

  const auto& readers = F3DReaderFactory::GetInstance()->GetReaders();
  if (readers.empty())
  {
    f3d::log::warn("No registered reader found!");
    return;
  }
  // Compute the size of the 3 columns
  for (const auto& reader : readers)
  {
    // sanity check
    if (reader->GetExtensions().size() < reader->GetMimeTypes().size())
    {
      f3d::log::error(reader->GetName(), " have different extensions and mime-types count.");
      return;
    }

    nameColSize = std::max(nameColSize, reader->GetName().length());
    descColSize = std::max(descColSize, reader->GetLongDescription().length());

    for (const auto& ext : reader->GetExtensions())
    {
      extsColSize = std::max(extsColSize, ext.length());
    }
    for (const auto& mime : reader->GetMimeTypes())
    {
      mimeColSize = std::max(mimeColSize, mime.length());
    }
  }
  nameColSize++;
  extsColSize++;
  mimeColSize++;
  descColSize++;

  std::string separator = std::string(nameColSize + extsColSize + descColSize + mimeColSize, '-');

  // Print the rows split in 3 columns
  std::stringstream headerLine;
  headerLine << std::left << std::setw(nameColSize) << "Name" << std::setw(descColSize)
             << "Description" << std::setw(extsColSize) << "Exts" << std::setw(mimeColSize)
             << "Mime-types";
  f3d::log::info(headerLine.str());
  f3d::log::info(separator);

  for (const auto& reader : readers)
  {
    for (size_t i = 0; i < reader->GetExtensions().size(); i++)
    {
      std::stringstream readerLine;
      if (i == 0)
      {
        readerLine << std::left << std::setw(nameColSize) << reader->GetName()
                   << std::setw(descColSize) << reader->GetLongDescription();
      }
      else
      {
        readerLine << std::left << std::setw(nameColSize + descColSize) << " ";
      }

      readerLine << std::setw(extsColSize) << reader->GetExtensions()[i];

      if (i < reader->GetMimeTypes().size())
      {
        readerLine << std::setw(mimeColSize) << reader->GetMimeTypes()[i];
      }

      f3d::log::info(readerLine.str());
    }
    f3d::log::info(separator);
  }
  f3d::log::waitForUser();
}
}
