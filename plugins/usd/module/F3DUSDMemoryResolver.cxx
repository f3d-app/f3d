#include "F3DUSDMemoryResolver.h"

#include <vtkResourceStream.h>

#include <pxr/usd/ar/defineResolver.h>
#include <pxr/usd/ar/inMemoryAsset.h>

// Register the resolver
PXR_NAMESPACE_OPEN_SCOPE
AR_DEFINE_RESOLVER(F3DUSDMemoryResolver, ArResolver);
PXR_NAMESPACE_CLOSE_SCOPE

//----------------------------------------------------------------------------
std::string F3DUSDMemoryResolver::_CreateIdentifier(
  const std::string& assetPath, const PXR_NS::ArResolvedPath&) const
{
  return assetPath;
}

//----------------------------------------------------------------------------
std::string F3DUSDMemoryResolver::_CreateIdentifierForNewAsset(
  const std::string&, const PXR_NS::ArResolvedPath&) const
{
  // unreachable
  return {};
}

//----------------------------------------------------------------------------
PXR_NS::ArResolvedPath F3DUSDMemoryResolver::_Resolve(const std::string& assetPath) const
{
  return PXR_NS::ArResolvedPath(assetPath);
}

//----------------------------------------------------------------------------
PXR_NS::ArResolvedPath F3DUSDMemoryResolver::_ResolveForNewAsset(const std::string&) const
{
  // unreachable
  return {};
}

//----------------------------------------------------------------------------
std::shared_ptr<PXR_NS::ArAsset> F3DUSDMemoryResolver::_OpenAsset(
  const PXR_NS::ArResolvedPath&) const
{
  vtkResourceStream* stream = _GetCurrentContextObject<F3DUSDMemoryResolverContext>()->Stream;

  // Read stream contents into buffer
  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  const std::size_t size = stream->Tell();
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  const std::shared_ptr<char> buffer(new char[size], std::default_delete<char[]>());

  stream->Read(buffer.get(), size);

  return PXR_NS::ArInMemoryAsset::FromBuffer(buffer, size);
}

//----------------------------------------------------------------------------
std::shared_ptr<PXR_NS::ArWritableAsset> F3DUSDMemoryResolver::_OpenAssetForWrite(
  const PXR_NS::ArResolvedPath&, WriteMode) const
{
  // unreachable
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
