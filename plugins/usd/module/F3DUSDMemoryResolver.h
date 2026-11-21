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
class F3DUSDMemoryResolver final : public PXR_NS::ArResolver
{
protected:
  /**
   * Returns assetPath as the identifier, ignoring anchorAssetPath since it is not relevant for this
   * resolver.
   */
  std::string _CreateIdentifier(
    const std::string& assetPath, const PXR_NS::ArResolvedPath& anchorAssetPath) const override;

  /**
   * Returns a resolved path copying assetPath as is.
   */
  PXR_NS::ArResolvedPath _Resolve(const std::string& assetPath) const override;

  /**
   * Function called by usd to open assets, implemented to read data from a vtkResourceStream
   */
  std::shared_ptr<PXR_NS::ArAsset> _OpenAsset(
    const PXR_NS::ArResolvedPath& resolvedPath) const override;

  //@{
  /**
   * Implements dummy pure virtual functions
   */
  std::string _CreateIdentifierForNewAsset(
    const std::string& assetPath, const PXR_NS::ArResolvedPath& anchorAssetPath) const override;

  PXR_NS::ArResolvedPath _ResolveForNewAsset(const std::string& assetPath) const override;

  std::shared_ptr<PXR_NS::ArWritableAsset> _OpenAssetForWrite(
    const PXR_NS::ArResolvedPath& resolvedPath, WriteMode writeMode) const override;
  //@}
};

/**
 * The context object used by the F3DUSDMemoryResolver to pass the vtkResourceStream.
 */
struct F3DUSDMemoryResolverContext
{
  vtkResourceStream* Stream = nullptr;

  bool operator<(const F3DUSDMemoryResolverContext&) const;
  bool operator==(const F3DUSDMemoryResolverContext&) const;
};

size_t hash_value(const F3DUSDMemoryResolverContext&);

PXR_NAMESPACE_OPEN_SCOPE
template<>
struct ArIsContextObject<F3DUSDMemoryResolverContext>
{
  static const bool value = true;
};
PXR_NAMESPACE_CLOSE_SCOPE

#endif
