/**
 * @class   F3DMemoryResolver
 * @brief   Custom ArResolver for loading USD assets from memory buffers
 *
 * Handles the "f3dmem:" URI scheme.
 */

#pragma once

#include <pxr/usd/ar/resolver.h>

class vtkResourceStream;

class F3DMemoryResolver final : public pxr::ArResolver
{
public:
  static vtkResourceStream* ActiveStream;

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
