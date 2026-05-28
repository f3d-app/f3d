#include "F3DUSDResolver.h"

#include <vtkResourceStream.h>

#include <pxr/base/arch/symbols.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/usd/ar/asset.h>
#include <pxr/usd/ar/defineResolver.h>
#include <pxr/usd/ar/inMemoryAsset.h>
#include <pxr/usd/ar/resolvedPath.h>
#include <pxr/usd/ar/resolverContext.h>

#include <cassert>

vtkResourceStream* F3DMemoryResolver::ActiveStream = nullptr;

//----------------------------------------------------------------------------
void F3DRegisterMemoryResolver()
{
  static bool registered = false;
  if (!registered)
  {
    registered = true;
    std::string libraryPath;
    if (!pxr::ArchGetAddressInfo(reinterpret_cast<void*>(&F3DRegisterMemoryResolver), &libraryPath,
          nullptr, nullptr, nullptr))
    {
      return;
    }

    std::string plugInfoDir = pxr::TfGetPathName(libraryPath) + "usd/resources/";
    pxr::PlugRegistry::GetInstance().RegisterPlugins(plugInfoDir);
  }
}

// Register the resolver
// TODO: can we avoid using namespace?
using namespace pxr;
AR_DEFINE_RESOLVER(F3DMemoryResolver, ArResolver);

//----------------------------------------------------------------------------
std::string F3DMemoryResolver::_CreateIdentifier(
  const std::string& assetPath, const pxr::ArResolvedPath&) const
{
  return assetPath;
}

//----------------------------------------------------------------------------
std::string F3DMemoryResolver::_CreateIdentifierForNewAsset(
  const std::string& assetPath, const pxr::ArResolvedPath&) const
{
  return assetPath;
}

//----------------------------------------------------------------------------
pxr::ArResolvedPath F3DMemoryResolver::_Resolve(const std::string& assetPath) const
{
  return pxr::ArResolvedPath(assetPath);
}

//----------------------------------------------------------------------------
pxr::ArResolvedPath F3DMemoryResolver::_ResolveForNewAsset(const std::string& assetPath) const
{
  return pxr::ArResolvedPath(assetPath);
}

//----------------------------------------------------------------------------
std::shared_ptr<pxr::ArAsset> F3DMemoryResolver::_OpenAsset(
  const pxr::ArResolvedPath& resolvedPath) const
{
  assert(F3DMemoryResolver::ActiveStream != nullptr);

  // Read stream contents into buffer
  F3DMemoryResolver::ActiveStream->Seek(0, vtkResourceStream::SeekDirection::End);
  const std::size_t size = F3DMemoryResolver::ActiveStream->Tell();
  F3DMemoryResolver::ActiveStream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  std::shared_ptr<char> buffer(new char[size], std::default_delete<char[]>());

  F3DMemoryResolver::ActiveStream->Read(buffer.get(), size);

  return ArInMemoryAsset::FromBuffer(buffer, size);
}

//----------------------------------------------------------------------------
bool F3DMemoryResolver::_CanWriteAssetToPath(const pxr::ArResolvedPath&, std::string* whyNot) const
{
  if (whyNot)
  {
    *whyNot = "F3DMemoryResolver is read-only";
  }
  return false;
}

//----------------------------------------------------------------------------
std::shared_ptr<pxr::ArWritableAsset> F3DMemoryResolver::_OpenAssetForWrite(
  const pxr::ArResolvedPath&, WriteMode) const
{
  return nullptr;
}
