#include "F3DUSDMemoryResolver.h"

#include <vtkResourceStream.h>

#include <pxr/usd/ar/defineResolver.h>
#include <pxr/usd/ar/inMemoryAsset.h>

// Register the resolver
namespace pxr
{
AR_DEFINE_RESOLVER(F3DUSDMemoryResolver, ArResolver);
}

//----------------------------------------------------------------------------
std::string F3DUSDMemoryResolver::_CreateIdentifier(
  const std::string& assetPath, const pxr::ArResolvedPath&) const
{
  return assetPath;
}

//----------------------------------------------------------------------------
std::string F3DUSDMemoryResolver::_CreateIdentifierForNewAsset(
  const std::string& assetPath, const pxr::ArResolvedPath&) const
{
  return assetPath;
}

//----------------------------------------------------------------------------
pxr::ArResolvedPath F3DUSDMemoryResolver::_Resolve(const std::string& assetPath) const
{
  return pxr::ArResolvedPath(assetPath);
}

//----------------------------------------------------------------------------
pxr::ArResolvedPath F3DUSDMemoryResolver::_ResolveForNewAsset(const std::string& assetPath) const
{
  return pxr::ArResolvedPath(assetPath);
}

//----------------------------------------------------------------------------
std::shared_ptr<pxr::ArAsset> F3DUSDMemoryResolver::_OpenAsset(const pxr::ArResolvedPath&) const
{
  vtkResourceStream* stream = _GetCurrentContextObject<F3DUSDMemoryResolverContext>()->Stream;

  // Read stream contents into buffer
  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  const std::size_t size = stream->Tell();
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  std::shared_ptr<char> buffer(new char[size], std::default_delete<char[]>());

  stream->Read(buffer.get(), size);

  return pxr::ArInMemoryAsset::FromBuffer(buffer, size);
}

//----------------------------------------------------------------------------
bool F3DUSDMemoryResolver::_CanWriteAssetToPath(const pxr::ArResolvedPath&, std::string* whyNot) const
{
  if (whyNot)
  {
    *whyNot = "F3DUSDMemoryResolver is read-only";
  }
  return false;
}

//----------------------------------------------------------------------------
std::shared_ptr<pxr::ArWritableAsset> F3DUSDMemoryResolver::_OpenAssetForWrite(
  const pxr::ArResolvedPath&, WriteMode) const
{
  return nullptr;
}

//----------------------------------------------------------------------------
bool F3DUSDMemoryResolverContext::operator<(const F3DUSDMemoryResolverContext& o) const
{
  return this->Stream < o.Stream;
}

//----------------------------------------------------------------------------
bool F3DUSDMemoryResolverContext::operator==(const F3DUSDMemoryResolverContext& o) const
{
  return this->Stream == o.Stream;
}

//----------------------------------------------------------------------------
size_t hash_value(const F3DUSDMemoryResolverContext& c)
{
  return std::hash<void*>{}(c.Stream);
}
