// Clip Library
// Copyright (c) 2020-2024 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip_win_wic.h"

#include "clip.h"

#include <algorithm>
#include <vector>

#include <shlwapi.h>
#include <wincodec.h>

namespace clip {
namespace win {

namespace {

// Successful calls to CoInitialize() (S_OK or S_FALSE) must match
// the calls to CoUninitialize().
// From: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize#remarks
struct coinit {
  HRESULT hr;
  coinit() {
    hr = CoInitialize(nullptr);
  }
  ~coinit() {
    if (hr == S_OK || hr == S_FALSE)
      CoUninitialize();
  }
};

template<class T>
class comptr {
public:
  comptr() { }
  explicit comptr(T* ptr) : m_ptr(ptr) { }
  comptr(const comptr&) = delete;
  comptr& operator=(const comptr&) = delete;
  ~comptr() { reset(); }

  T** operator&() { return &m_ptr; }
  T* operator->() { return m_ptr; }
  bool operator!() const { return !m_ptr; }

  T* get() { return m_ptr; }
  void reset() {
    if (m_ptr) {
      m_ptr->Release();
      m_ptr = nullptr;
    }
  }
private:
  T* m_ptr = nullptr;
};

#ifdef CLIP_SUPPORT_WINXP
class hmodule {
public:
  hmodule(LPCWSTR name) : m_ptr(LoadLibraryW(name)) { }
  hmodule(const hmodule&) = delete;
  hmodule& operator=(const hmodule&) = delete;
  ~hmodule() {
    if (m_ptr)
      FreeLibrary(m_ptr);
  }

  operator HMODULE() { return m_ptr; }
  bool operator!() const { return !m_ptr; }
private:
  HMODULE m_ptr = nullptr;
};
#endif

struct WicImageFormat {
  const char* names[3];        // Alternative names of this format
  UINT ids[3];                 // Clipboard format ID for each name of this format
  ReadWicImageFormatFunc read; // Function used to decode data in this format
};

WicImageFormat wic_image_formats[] = {
  { { "PNG", "image/png",  nullptr }, { 0, 0, 0 }, read_png },
  { { "JPG", "image/jpeg", "JPEG"  }, { 0, 0, 0 }, read_jpg },
  { { "BMP", "image/bmp",  nullptr }, { 0, 0, 0 }, read_bmp },
  { { "GIF", "image/gif",  nullptr }, { 0, 0, 0 }, read_gif }
};

} // anonymous namespace

ReadWicImageFormatFunc wic_image_format_available(UINT* output_cbformat) {
  for (auto& fmt : wic_image_formats) {
    for (int i=0; i<3; ++i) {
      const char* name = fmt.names[i];
      if (!name)
        break;

      // Although RegisterClipboardFormatA() already returns the same
      // value for the same "name" (even for different apps), we
      // prefer to cache the value to avoid calling
      // RegisterClipboardFormatA() several times (as internally that
      // function must do some kind of hash map name -> ID
      // conversion).
      UINT cbformat = fmt.ids[i];
      if (cbformat == 0)
        fmt.ids[i] = cbformat = RegisterClipboardFormatA(name);

      if (cbformat && IsClipboardFormatAvailable(cbformat)) {
        if (output_cbformat)
          *output_cbformat = cbformat;
        return fmt.read;
      }
    }
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////
// Encode the image as PNG format

bool write_png_on_stream(const image& image,
                         IStream* stream) {
  const image_spec& spec = image.spec();

  comptr<IWICBitmapEncoder> encoder;
  HRESULT hr = CoCreateInstance(CLSID_WICPngEncoder,
                                nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&encoder));
  if (FAILED(hr))
    return false;

  hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
  if (FAILED(hr))
    return false;

  comptr<IWICBitmapFrameEncode> frame;
  comptr<IPropertyBag2> options;
  hr = encoder->CreateNewFrame(&frame, &options);
  if (FAILED(hr))
    return false;

  hr = frame->Initialize(options.get());
  if (FAILED(hr))
    return false;

  // PNG encoder (and decoder) only supports GUID_WICPixelFormat32bppBGRA for 32bpp.
  // See: https://docs.microsoft.com/en-us/windows/win32/wic/-wic-codec-native-pixel-formats#png-native-codec
  WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppBGRA;
  hr = frame->SetPixelFormat(&pixelFormat);
  if (FAILED(hr))
    return false;

  hr = frame->SetSize(spec.width, spec.height);
  if (FAILED(hr))
    return false;

  std::vector<uint32_t> buf;
  uint8_t* ptr = (uint8_t*)image.data();
  int bytes_per_row = spec.bytes_per_row;

  // Convert to GUID_WICPixelFormat32bppBGRA if needed
  if (spec.red_mask != 0xff0000 ||
      spec.green_mask != 0xff00 ||
      spec.blue_mask != 0xff ||
      spec.alpha_mask != 0xff000000) {
    buf.resize(spec.width * spec.height);
    uint32_t* dst = (uint32_t*)&buf[0];
    uint32_t* src = (uint32_t*)image.data();
    for (int y=0; y<spec.height; ++y) {
      auto src_line_start = src;
      for (int x=0; x<spec.width; ++x) {
        uint32_t c = *src;
        *dst = ((((c & spec.red_mask  ) >> spec.red_shift  ) << 16) |
                (((c & spec.green_mask) >> spec.green_shift) <<  8) |
                (((c & spec.blue_mask ) >> spec.blue_shift )      ) |
                (((c & spec.alpha_mask) >> spec.alpha_shift) << 24));
        ++dst;
        ++src;
      }
      src = (uint32_t*)(((uint8_t*)src_line_start) + spec.bytes_per_row);
    }
    ptr = (uint8_t*)&buf[0];
    bytes_per_row = 4 * spec.width;
  }

  hr = frame->WritePixels(spec.height,
                          bytes_per_row,
                          bytes_per_row * spec.height,
                          (BYTE*)ptr);
  if (FAILED(hr))
    return false;

  hr = frame->Commit();
  if (FAILED(hr))
    return false;

  hr = encoder->Commit();
  if (FAILED(hr))
    return false;

  return true;
}

HGLOBAL write_png(const image& image) {
  coinit com;

  comptr<IStream> stream;
  HRESULT hr = CreateStreamOnHGlobal(nullptr, false, &stream);
  if (FAILED(hr))
    return nullptr;

  bool result = write_png_on_stream(image, stream.get());

  HGLOBAL handle;
  hr = GetHGlobalFromStream(stream.get(), &handle);
  if (result)
    return handle;

  GlobalFree(handle);
  return nullptr;
}

IStream* create_stream(const BYTE* pInit, UINT cbInit)
{
#ifdef CLIP_SUPPORT_WINXP
  // Pull SHCreateMemStream from shlwapi.dll by ordinal 12
  // for Windows XP support
  // From: https://learn.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-shcreatememstream#remarks

  typedef IStream*(WINAPI * SHCreateMemStreamPtr)(const BYTE* pInit,
                                                  UINT cbInit);
  hmodule shlwapiDll(L"shlwapi.dll");
  if (!shlwapiDll)
    return false;

  auto SHCreateMemStream = reinterpret_cast<SHCreateMemStreamPtr>(
    GetProcAddress(shlwapiDll, (LPCSTR)12));
  if (!SHCreateMemStream)
    return false;
#endif
  return SHCreateMemStream(pInit, cbInit);
}

image_spec spec_from_pixelformat(const WICPixelFormatGUID& pixelFormat, unsigned long w, unsigned long h)
{
  image_spec spec;
  spec.width = w;
  spec.height = h;
  if (pixelFormat == GUID_WICPixelFormat32bppBGRA ||
      pixelFormat == GUID_WICPixelFormat32bppBGR) {
    spec.bits_per_pixel = 32;
    spec.red_mask = 0xff0000;
    spec.green_mask = 0xff00;
    spec.blue_mask = 0xff;
    spec.alpha_mask = 0xff000000;
    spec.red_shift = 16;
    spec.green_shift = 8;
    spec.blue_shift = 0;
    spec.alpha_shift = 24;
    // Reset mask and shift for BGR pixel format.
    if (pixelFormat == GUID_WICPixelFormat32bppBGR) {
      spec.alpha_mask = 0;
      spec.alpha_shift = 0;
    }
  }
  else if (pixelFormat == GUID_WICPixelFormat24bppBGR ||
           pixelFormat == GUID_WICPixelFormat8bppIndexed) {
    spec.bits_per_pixel = 24;
    spec.red_mask = 0xff0000;
    spec.green_mask = 0xff00;
    spec.blue_mask = 0xff;
    spec.alpha_mask = 0;
    spec.red_shift = 16;
    spec.green_shift = 8;
    spec.blue_shift = 0;
    spec.alpha_shift = 0;
  }
  spec.bytes_per_row = ((w*spec.bits_per_pixel+31) / 32) * 4;
  return spec;
}

// Tries to decode the input buf of size len using the specified
// decoders. If output_image is not null, the decoded image is
// returned there, if output_spec is not null then the image
// specifications are set there.
bool decode(const GUID decoder_clsid1,
            const GUID decoder_clsid2,
            const uint8_t* buf,
            const UINT len,
            image* output_image,
            image_spec* output_spec)
{
  coinit com;

  comptr<IWICBitmapDecoder> decoder;
  HRESULT hr = CoCreateInstance(decoder_clsid1, nullptr,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&decoder));
  if (FAILED(hr) && decoder_clsid2 != GUID_NULL) {
    hr = CoCreateInstance(decoder_clsid2, nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&decoder));
  }
  if (FAILED(hr))
    return false;

  // Can decoder be nullptr if hr is S_OK/successful? We've received
  // some crash reports that might indicate this.
  if (!decoder)
    return false;

  comptr<IStream> stream(create_stream(buf, len));
  if (!stream)
    return false;

  hr = decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand);
  if (FAILED(hr))
    return false;

  comptr<IWICBitmapFrameDecode> frame;
  hr = decoder->GetFrame(0, &frame);
  if (FAILED(hr))
    return false;

  WICPixelFormatGUID pixelFormat;
  hr = frame->GetPixelFormat(&pixelFormat);
  if (FAILED(hr))
    return false;

  // Only support these pixel formats
  // TODO add support for more pixel formats
  if (pixelFormat != GUID_WICPixelFormat32bppBGRA &&
      pixelFormat != GUID_WICPixelFormat32bppBGR &&
      pixelFormat != GUID_WICPixelFormat24bppBGR &&
      pixelFormat != GUID_WICPixelFormat8bppIndexed)
    return false;

  UINT width = 0, height = 0;
  hr = frame->GetSize(&width, &height);
  if (FAILED(hr))
    return false;

  image_spec spec = spec_from_pixelformat(pixelFormat, width, height);

  if (output_spec)
    *output_spec = spec;

  image img;
  if (output_image) {
    if (pixelFormat == GUID_WICPixelFormat8bppIndexed) {
      std::vector<BYTE> pixels(spec.width * spec.height);
      hr = frame->CopyPixels(nullptr,  // Entire bitmap
                             spec.width,
                             spec.width * spec.height,
                             pixels.data());

      if (FAILED(hr))
        return false;

      comptr<IWICImagingFactory> factory;
      HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory,
                                    nullptr,
                                    CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARGS(&factory));
      if (FAILED(hr))
        return false;

      comptr<IWICPalette> palette;
      hr = factory->CreatePalette(&palette);
      if (FAILED(hr))
        return false;

      hr = frame->CopyPalette(palette.get());
      if (FAILED(hr))
        return false;

      UINT numcolors;
      hr = palette->GetColorCount(&numcolors);
      if (FAILED(hr))
        return false;

      UINT actualNumcolors;
      std::vector<WICColor> colors(numcolors);
      hr = palette->GetColors(numcolors, colors.data(), &actualNumcolors);
      if (FAILED(hr))
        return false;

      BOOL hasAlpha = false;
      palette->HasAlpha(&hasAlpha);
      if (hasAlpha) {
        spec = spec_from_pixelformat(GUID_WICPixelFormat32bppBGRA, width, height);
      }

      img = image(spec);
      char* dst = img.data();
      BYTE* src = pixels.data();
      for (int y = 0; y < spec.height; ++y) {
        char* dst_x = dst;
        for (int x = 0; x < spec.width; ++x, dst_x+=spec.bits_per_pixel/8, ++src) {
          *((uint32_t*)dst_x) = (*src < numcolors ? colors[*src] : 0);
        }
        dst += spec.bytes_per_row;
      }
    }
    else {
      img = image(spec);
      hr = frame->CopyPixels(nullptr,  // Entire bitmap
                             spec.bytes_per_row,
                             spec.bytes_per_row * spec.height,
                             (BYTE*)img.data());
      if (FAILED(hr))
        return false;
    }


    std::swap(*output_image, img);
  }

  return true;
}

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from PNG format

bool read_png(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec) {
  return decode(CLSID_WICPngDecoder2, CLSID_WICPngDecoder1,
                buf, len, output_image, output_spec);
}

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from JPEG format

bool read_jpg(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec)
{
  return decode(CLSID_WICJpegDecoder, GUID_NULL,
                buf, len, output_image, output_spec);
}

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from GIF format

bool read_gif(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec)
{
  return decode(CLSID_WICGifDecoder, GUID_NULL,
                buf, len, output_image, output_spec);
}

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from BMP format

bool read_bmp(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec)
{
  return decode(CLSID_WICBmpDecoder, GUID_NULL,
                buf, len, output_image, output_spec);
}

} // namespace win
} // namespace clip
