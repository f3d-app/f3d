#include "F3DThumbnailProvider.h"
#include "F3DThumbnailConfig.h"

#include <clocale>
#include <codecvt>
#include <locale>
#include <pathcch.h>
#include <shlwapi.h>
#include <sstream>
#include <thumbcache.h>
#include <wincodec.h>

extern HINSTANCE g_hInst;
extern long g_cDllRef;

namespace
{
//------------------------------------------------------------------------------
HRESULT ConvertBitmapSourceTo32BPPHBITMAP(
  IWICBitmapSource* pBitmapSource, IWICImagingFactory* pImagingFactory, HBITMAP* phbmp)
{
  IWICBitmapSource* pBitmapSourceConverted = nullptr;
  WICPixelFormatGUID guidPixelFormatSource;

  *phbmp = nullptr;
  HRESULT hr = pBitmapSource->GetPixelFormat(&guidPixelFormatSource);

  if (SUCCEEDED(hr) && (guidPixelFormatSource != GUID_WICPixelFormat32bppBGRA))
  {
    IWICFormatConverter* pFormatConverter;
    hr = pImagingFactory->CreateFormatConverter(&pFormatConverter);
    if (SUCCEEDED(hr))
    {
      // Create the appropriate pixel format converter
      hr = pFormatConverter->Initialize(pBitmapSource, GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
      if (SUCCEEDED(hr))
      {
        hr = pFormatConverter->QueryInterface(&pBitmapSourceConverted);
      }
      pFormatConverter->Release();
    }
  }
  else
  {
    // Conversion not necessary
    hr = pBitmapSource->QueryInterface(&pBitmapSourceConverted);
  }

  if (!SUCCEEDED(hr))
  {
    return hr;
  }

  UINT nWidth, nHeight;
  hr = pBitmapSourceConverted->GetSize(&nWidth, &nHeight);
  if (SUCCEEDED(hr))
  {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = nWidth;
    bmi.bmiHeader.biHeight = -static_cast<LONG>(nHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    BYTE* pBits;
    HBITMAP hbmp =
      CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&pBits), nullptr, 0);
    hr = hbmp ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
      WICRect rect = { 0, 0, static_cast<INT>(nWidth), static_cast<INT>(nHeight) };

      // Convert the pixels and store them in the HBITMAP.
      hr = pBitmapSourceConverted->CopyPixels(&rect, nWidth * 4, nWidth * nHeight * 4, pBits);
      if (SUCCEEDED(hr))
      {
        *phbmp = hbmp;
      }
      else
      {
        DeleteObject(hbmp);
      }
    }
  }
  pBitmapSourceConverted->Release();
  return hr;
}
}

//------------------------------------------------------------------------------
F3DThumbnailProvider::F3DThumbnailProvider()
  : m_cRef(1)
  , m_filePath(L"")
{
  ::InterlockedIncrement(&g_cDllRef);

  // Get the f3d executable path based on the location of this dll
  wchar_t dll_path[MAX_PATH];
  if (GetModuleFileName(g_hInst, dll_path, ARRAYSIZE(dll_path)) != 0)
  {
    ::PathCchRemoveFileSpec(dll_path, MAX_PATH);
    PathCchCombine(m_f3dPath, MAX_PATH, dll_path, L"f3d.exe");
  }
}

//------------------------------------------------------------------------------
F3DThumbnailProvider::~F3DThumbnailProvider()
{
  ::InterlockedDecrement(&g_cDllRef);
}

//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) F3DThumbnailProvider::AddRef()
{
  return ::InterlockedIncrement(&m_cRef);
}

//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) F3DThumbnailProvider::Release()
{
  ULONG cRef = ::InterlockedDecrement(&m_cRef);
  if (cRef == 0)
  {
    delete this;
  }
  return cRef;
}

//------------------------------------------------------------------------------
// IInitializeWithFile
// Provides the path to the file for which we will have to produce a thumbnail.
IFACEMETHODIMP F3DThumbnailProvider::Initialize(LPCWSTR pszFilePath, DWORD)
{
  HRESULT hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
  if (m_filePath[0] == L'\0')
  {
    hr = wcscpy_s(m_filePath, pszFilePath);
  }
  return hr;
}

//------------------------------------------------------------------------------
// IThumbnailProvider
// Generate the thumbnail bitmap for the requested file.
IFACEMETHODIMP F3DThumbnailProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
  // Get a temporary PNG image file name
  wchar_t lpTempPathBuffer[MAX_PATH];
  wchar_t image_filename[MAX_PATH];

  DWORD dwRetVal = GetTempPathW(MAX_PATH, lpTempPathBuffer);
  if (dwRetVal > MAX_PATH || (dwRetVal == 0))
  {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  if (GetTempFileName(lpTempPathBuffer, L"f3d", 0, image_filename) == 0)
  {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  wcscat_s(image_filename, L".png");

  // Create command to run
  wchar_t command[MAX_PATH * 3 + 20];
  swprintf_s(command, MAX_PATH * 3 + 20,
    L"\"%s\" --input \"%s\" --output \"%s\" --config=thumbnail --quiet --resolution "
    L"%d,%d\"",
    m_f3dPath, m_filePath, image_filename, cx, cx);

  // Let's launch the process
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

  STARTUPINFO si;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags |= STARTF_USESTDHANDLES;

  if (!CreateProcess(
        m_f3dPath, command, nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
  {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  DWORD waitCode = WaitForSingleObject(pi.hProcess, F3D_WINDOWS_THUMBNAIL_TIMEOUT);

  // Clean up
  DWORD exitCode = EXIT_SUCCESS;
  GetExitCodeProcess(pi.hProcess, &exitCode);

  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  // Return here if f3d failed
  if (waitCode != WAIT_OBJECT_0 || exitCode != EXIT_SUCCESS)
  {
    DeleteFile(image_filename);
    return E_FAIL;
  }

  // Load the created image

  // Create WIC factory
  IWICImagingFactory* pIWICFactory = nullptr;
  HRESULT hr = CoCreateInstance(
    CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));

  if (FAILED(hr))
  {
    DeleteFile(image_filename);
    return hr;
  }

  // Create image decoder
  IWICBitmapDecoder* pDecoder = nullptr;
  hr = pIWICFactory->CreateDecoderFromFilename(
    LPCWSTR(image_filename), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);

  if (FAILED(hr))
  {
    pIWICFactory->Release();
    DeleteFile(image_filename);
    return hr;
  }

  // Load the first image frame
  IWICBitmapFrameDecode* pFrame = nullptr;
  hr = pDecoder->GetFrame(0, &pFrame);

  if (FAILED(hr))
  {
    pDecoder->Release();
    pIWICFactory->Release();
    DeleteFile(image_filename);
    return hr;
  }

  // Convert to 32bpp BGRA format with pre-multiplied alpha
  hr = ::ConvertBitmapSourceTo32BPPHBITMAP(pFrame, pIWICFactory, phbmp);
  *pdwAlpha = WTSAT_ARGB;

  pFrame->Release();
  pDecoder->Release();
  pIWICFactory->Release();

  // Delete the temporary image file
  if (!DeleteFile(image_filename))
  {
    return E_FAIL;
  }

  return hr;
}
