#pragma once

#include <cstdio>
#include <new>
#include <shlwapi.h>
#include <thumbcache.h>
#include <wincodec.h>
#include <windows.h>

// Class to provide thumbnails using F3D
// This is using a standard windows API
class F3DThumbnailProvider
  : public IInitializeWithFile
  , public IThumbnailProvider
{
public:
  F3DThumbnailProvider();

  // IUnknown
  IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
  {
    static const QITAB qit[] = {
      QITABENT(F3DThumbnailProvider, IThumbnailProvider),
      QITABENT(F3DThumbnailProvider, IInitializeWithFile),
      { 0 },
    };
    return QISearch(this, qit, riid, ppv);
  }

  IFACEMETHODIMP_(ULONG) AddRef();
  IFACEMETHODIMP_(ULONG) Release();

  // IInitializeWithFile
  IFACEMETHODIMP Initialize(LPCWSTR pszFilePath, DWORD grfMode);

  // IThumbnailProvider
  IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);

protected:
  ~F3DThumbnailProvider();

private:
  long m_cRef;

  // The path to f3d executable that will be used to produce the thumbnail
  wchar_t m_f3dPath[MAX_PATH];

  // The path to the file for which we will have to produce the thumbnail
  wchar_t m_filePath[MAX_PATH];
};
