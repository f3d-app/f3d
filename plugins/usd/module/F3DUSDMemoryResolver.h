/**
 * @class   F3DUSDMemoryResolver
 * @brief   Custom ArResolver for loading USD assets from memory buffers
 *
 * Handles the "f3dmem" URI scheme.
 * When opening an asset using a path like "f3dmem:stream.usdc", this resolver is used
 * to read the asset's contents from a VTK stream that is passed through the resolver context.
 */
#ifndef F3DUSDMemoryResolver_h
#define F3DUSDMemoryResolver_h

#include <pxr/usd/ar/resolver.h>

class vtkResourceStream;

/**
 * The actual resolver class that implements the ArResolver interface.
 */
class F3DUSDMemoryResolver final : public pxr::ArResolver
{
protected:
  // implements pure virtual functions
  std::string _CreateIdentifier(
    const std::string& assetPath, const pxr::ArResolvedPath& anchorAssetPath) const override;

  std::string _CreateIdentifierForNewAsset(
    const std::string& assetPath, const pxr::ArResolvedPath& anchorAssetPath) const override;

  pxr::ArResolvedPath _Resolve(const std::string& assetPath) const override;

  pxr::ArResolvedPath _ResolveForNewAsset(const std::string& assetPath) const override;

  // this is the function that actually gets called to open assets
  std::shared_ptr<pxr::ArAsset> _OpenAsset(const pxr::ArResolvedPath& resolvedPath) const override;

  // signals that this resolver cannot write any assets
  bool _CanWriteAssetToPath(
    const pxr::ArResolvedPath& resolvedPath, std::string* whyNot) const override;

  std::shared_ptr<pxr::ArWritableAsset> _OpenAssetForWrite(
    const pxr::ArResolvedPath& resolvedPath, WriteMode writeMode) const override;
};

/**
 * The context object used by the F3DUSDMemoryResolver to pass the VTK stream.
 */
struct F3DUSDMemoryResolverContext
{
  vtkResourceStream* Stream = nullptr;

  bool operator<(const F3DUSDMemoryResolverContext&) const;
  bool operator==(const F3DUSDMemoryResolverContext&) const;
};

size_t hash_value(const F3DUSDMemoryResolverContext&);

template<>
struct pxr::ArIsContextObject<F3DUSDMemoryResolverContext>
{
  static const bool value = true;
};

#endif
