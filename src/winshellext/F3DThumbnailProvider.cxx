#include "F3DThumbnailProvider.h"

#include "F3DException.h"
#include "F3DLoader.h"
#include "vtkF3DObjectFactory.h"

#include <vtkImageData.h>
#include <vtkNew.h>

#include <clocale>
#include <codecvt>
#include <locale>
#include <pathcch.h>
#include <sstream>

extern HINSTANCE g_hInst;
extern long g_cDllRef;

namespace
{
//------------------------------------------------------------------------------
// Convert wide string command line to argc,argv
void ConvertWCommandLineToArgs(wchar_t* args, int &argc, char** &argv)
{
  std::setlocale(LC_ALL, "en_US.utf8");
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conversion;

  wchar_t** wargv = CommandLineToArgvW(args, &argc);
  argv = new char*[argc];
  for (int i = 0; i < argc; i++)
  {
    std::string mbs = conversion.to_bytes(wargv[i]);
    argv[i] = new char[mbs.size() + 1];
    std::copy(mbs.begin(), mbs.end(), argv[i]);
    argv[i][mbs.size()] = 0;
  }
}

//------------------------------------------------------------------------------
HRESULT CreateBitmapFromBuffer(vtkImageData* image, HBITMAP* phbmp)
{
  // Fill in image information.
  int* dims = image->GetDimensions();
  int width = dims[0];
  int height = dims[1];
  int depth = image->GetNumberOfScalarComponents();

  size_t rowLen = (depth == 4) ? (width * 4) : ((width * 3 + 3) & ~3);

  BITMAPINFO bi = {};
  ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = width;
  bi.bmiHeader.biHeight = height;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 8 * depth;
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = static_cast<DWORD>(rowLen * height);

  LPBYTE bitmapBuffer = nullptr;
  *phbmp = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void**)&bitmapBuffer, NULL, 0);

  void* imageBuffer = image->GetScalarPointer();
  if (rowLen == width * depth)
  {
    // Copy image buffer
    memcpy(bitmapBuffer, imageBuffer, width * height * depth);
    // Convert RGBA to BGRA
    for (int i = 0; i < width * height * depth; i += depth)
    {
      std::swap(bitmapBuffer[i], bitmapBuffer[i + 2]);
    }
  }
  else
  {
    LPBYTE bitmapPtr = bitmapBuffer;
    LPBYTE imagePtr = reinterpret_cast<LPBYTE>(imageBuffer);
    int lineLen = width * depth;
    for (int y = 0; y < height; y++)
    {
      memcpy(bitmapPtr, imagePtr, lineLen);
      // Convert RGBA to BGRA
      for (int x = 0; x < width * 3; x += 3) 
      {
        std::swap(bitmapPtr[x], bitmapPtr[x + 2]);
      }
      bitmapPtr += rowLen;
      imagePtr += lineLen;
    }
  }

  SetDIBits(NULL, *phbmp, 0, height, bitmapBuffer, &bi, DIB_RGB_COLORS);

  return NOERROR;
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
  HRESULT ret = E_FAIL;
  wchar_t warg[MAX_PATH * 2];
  swprintf_s(warg, MAX_PATH * 2,
    L"f3d --dry-run --quiet -sta --no-background --resolution=%d,%d \"%s\"",
    cx, cx, m_filePath);
  int argc;
  char** argv;
  ::ConvertWCommandLineToArgs(warg, argc, argv);

  try
  {
#if NDEBUG
    vtkObject::GlobalWarningDisplayOff();
#endif

    F3DLoader loader;
    vtkNew<vtkImageData> image;
    int r = loader.Start(argc, argv, image);
    UINT imgWidth = static_cast<UINT>(image->GetDimensions()[0]);

    if (imgWidth == cx && r == EXIT_SUCCESS)
    {
      ::CreateBitmapFromBuffer(image, phbmp);
      *pdwAlpha = WTSAT_ARGB;
      ret = NOERROR;
    }
  }
  catch (...)
  {
  }

  for (int i = 0; i < argc; i++)
  {  
    delete[] argv[i];
  }
  delete[] argv;

  return ret;
}
