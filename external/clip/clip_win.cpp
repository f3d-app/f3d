// Clip Library
// Copyright (C) 2015-2024  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip_win.h"

#include "clip.h"
#include "clip_lock_impl.h"
#include "clip_win_hglobal.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace clip {

using namespace win;

namespace {

// Data type used as header for custom formats to indicate the exact
// size of the user custom data. This is necessary because it looks
// like GlobalSize() might not return the exact size, but a greater
// value.
typedef uint64_t CustomSizeT;

// From: https://issues.chromium.org/issues/40080988#comment8
//
//  "Adds impersonation of the anonymous token around calls to the
//   CloseClipboard() system call. On Windows 8+ the win32k driver
//   captures the access token of the caller and makes it available to
//   other users on the desktop through the system call
//   GetClipboardAccessToken(). This introduces a risk of privilege
//   escalation in sandboxed processes. By performing the
//   impersonation then whenever Chrome writes data to the clipboard
//   only the anonymous token is available."
//
class AnonymousTokenImpersonator {
public:
  AnonymousTokenImpersonator()
    : m_must_revert(ImpersonateAnonymousToken(GetCurrentThread()))
  {}

  ~AnonymousTokenImpersonator() {
    if (m_must_revert)
      RevertToSelf();
  }
private:
  const bool m_must_revert;
};

} // anonymous namespace

lock::impl::impl(void* hwnd) : m_locked(false) {
  for (int i=0; i<5; ++i) {
    if (OpenClipboard((HWND)hwnd)) {
      m_locked = true;
      break;
    }
    Sleep(20);
  }

  if (!m_locked) {
    error_handler e = get_error_handler();
    if (e)
      e(ErrorCode::CannotLock);
  }
}

lock::impl::~impl() {
  if (m_locked) {
    AnonymousTokenImpersonator guard;
    CloseClipboard();
  }
}

bool lock::impl::clear() {
  return (EmptyClipboard() ? true: false);
}

bool lock::impl::is_convertible(format f) const {
  if (f == text_format()) {
    return
      (IsClipboardFormatAvailable(CF_TEXT) ||
       IsClipboardFormatAvailable(CF_UNICODETEXT) ||
       IsClipboardFormatAvailable(CF_OEMTEXT));
  }
#if CLIP_ENABLE_IMAGE
  else if (f == image_format()) {
    return (IsClipboardFormatAvailable(CF_DIB) ||
            win::wic_image_format_available(nullptr) != nullptr);
  }
#endif // CLIP_ENABLE_IMAGE
  else
    return IsClipboardFormatAvailable(f);
}

bool lock::impl::set_data(format f, const char* buf, size_t len) {
  bool result = false;

  if (f == text_format()) {
    if (len > 0) {
      int reqsize = MultiByteToWideChar(CP_UTF8, 0, buf, len, NULL, 0);
      if (reqsize > 0) {
        ++reqsize;

        Hglobal hglobal(sizeof(WCHAR)*reqsize);
        if (auto hlock = HglobalLock(hglobal)) {
          LPWSTR lpstr = hlock.data<LPWSTR>();
          MultiByteToWideChar(CP_UTF8, 0, buf, len, lpstr, reqsize);
        }

        result = hglobal.set_clipboard_data(CF_UNICODETEXT);
      }
    }
  }
  else {
    Hglobal hglobal(len+sizeof(CustomSizeT));
    if (auto hlock = HglobalLock(hglobal)) {
      auto dst = hlock.data<uint8_t*>();
      *((CustomSizeT*)dst) = len;
      memcpy(dst+sizeof(CustomSizeT), buf, len);
      result = hglobal.set_clipboard_data(f);
    }
  }

  return result;
}

bool lock::impl::get_data(format f, char* buf, size_t len) const {
  assert(buf);

  if (!buf || !is_convertible(f))
    return false;

  bool result = false;

  if (f == text_format()) {
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_UNICODETEXT);
      if (auto hlock = HglobalLock(hglobal)) {
        LPWSTR lpstr = hlock.data<LPWSTR>();
        size_t reqsize = WideCharToMultiByte(CP_UTF8, 0, lpstr, -1,
                                             nullptr, 0, nullptr, nullptr);

        assert(reqsize <= len);
        if (reqsize <= len) {
          WideCharToMultiByte(CP_UTF8, 0, lpstr, -1,
                              buf, reqsize, nullptr, nullptr);
          result = true;
        }
      }
    }
    else if (IsClipboardFormatAvailable(CF_TEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_TEXT);
      if (auto hlock = HglobalLock(hglobal)) {
        LPSTR lpstr = hlock.data<LPSTR>();
        // TODO check length
        memcpy(buf, lpstr, len);
        result = true;
      }
    }
  }
  else {
    if (IsClipboardFormatAvailable(f)) {
      HGLOBAL hglobal = GetClipboardData(f);
      if (auto hlock = HglobalLock(hglobal)) {
        const size_t total_size = GlobalSize(hglobal);
        auto ptr = hlock.data<const uint8_t*>();
        CustomSizeT reqsize = *((CustomSizeT*)ptr);

        // If the registered length of data in the first CustomSizeT
        // number of bytes of the hglobal data is greater than the
        // GlobalSize(hglobal), something is wrong, it should not
        // happen.
        assert(reqsize <= total_size);
        if (reqsize > total_size)
          reqsize = total_size - sizeof(CustomSizeT);

        if (reqsize <= len) {
          memcpy(buf, ptr+sizeof(CustomSizeT), reqsize);
          result = true;
        }
      }
    }
  }

  return result;
}

size_t lock::impl::get_data_length(format f) const {
  size_t len = 0;

  if (f == text_format()) {
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_UNICODETEXT);
      if (auto hlock = HglobalLock(hglobal)) {
        LPWSTR lpstr = hlock.data<LPWSTR>();
        len = WideCharToMultiByte(CP_UTF8, 0, lpstr, -1,
                                  nullptr, 0, nullptr, nullptr);
      }
    }
    else if (IsClipboardFormatAvailable(CF_TEXT)) {
      HGLOBAL hglobal = GetClipboardData(CF_TEXT);
      if (auto hlock = HglobalLock(hglobal)) {
        LPSTR lpstr = hlock.data<LPSTR>();
        len = std::strlen(lpstr) + 1;
      }
    }
  }
  else if (f != empty_format()) {
    if (IsClipboardFormatAvailable(f)) {
      HGLOBAL hglobal = GetClipboardData(f);
      if (auto hlock = HglobalLock(hglobal)) {
        const SIZE_T total_size = GlobalSize(hglobal);
        auto ptr = hlock.data<const uint8_t*>();
        len = *((CustomSizeT*)ptr);

        assert(len <= total_size);
        if (len > total_size)
          len = total_size - sizeof(CustomSizeT);
      }
    }
  }

  return len;
}

#if CLIP_ENABLE_LIST_FORMATS

std::vector<format_info> lock::impl::list_formats() const {
  static const char* standard_formats[CF_MAX] = {
    "", "CF_TEXT", "CF_BITMAP", "CF_METAFILEPICT",
    "CF_SYLK", "CF_DIF", "CF_TIFF", "CF_OEMTEXT",
    "CF_DIB", "CF_PALETTE", "CF_PENDATA", "CF_RIFF",
    "CF_WAVE", "CF_UNICODETEXT", "CF_ENHMETAFILE", "CF_HDROP",
    "CF_LOCALE", "CF_DIBV5"
  };

  std::vector<format_info> formats;
  std::vector<char> format_name(512);

  formats.reserve(CountClipboardFormats());

  UINT format_id = EnumClipboardFormats(0);
  while (format_id != 0) {
    if (format_id >= CF_TEXT && format_id < CF_MAX) {
      // Standard clipboard format
      formats.emplace_back(format_id, standard_formats[format_id]);
    }
    // Get user-defined format name
    else {
      int size = GetClipboardFormatNameA(
        format_id,
        format_name.data(),
        format_name.size());

      formats.emplace_back(format_id, std::string(format_name.data(), size));
    }

    format_id = EnumClipboardFormats(format_id);
  }

  return formats;
}

#endif // CLIP_ENABLE_LIST_FORMATS

#if CLIP_ENABLE_IMAGE

bool lock::impl::set_image(const image& image) {
  const image_spec& spec = image.spec();

  // Add the PNG clipboard format for images with alpha channel
  // (useful to communicate with some Windows programs that only use
  // alpha data from PNG clipboard format)
  if (spec.bits_per_pixel == 32 &&
      spec.alpha_mask) {
    UINT png_format = RegisterClipboardFormatA("PNG");
    if (png_format) {
      Hglobal png_handle(win::write_png(image));
      if (png_handle)
        png_handle.set_clipboard_data(png_format);
    }
  }

  Hglobal hmem(clip::win::create_dibv5(image));
  if (!hmem)
    return false;

  hmem.set_clipboard_data(CF_DIBV5);
  return true;
}

bool lock::impl::get_image(image& output_img) const {
  // Tries to get the first image format that can be read using WIC
  // ("PNG", "JPG", "GIF", etc).
  UINT cbformat;
  if (auto read_img = win::wic_image_format_available(&cbformat)) {
    HANDLE handle = GetClipboardData(cbformat);
    if (auto hlock = HglobalLock(handle)) {
      size_t size = GlobalSize(handle);
      uint8_t* data = hlock.data<uint8_t*>();
      bool result = read_img(data, size, &output_img, nullptr);
      if (result)
        return true;
    }
  }

  // If we couldn't find any, we try to use the regular DIB format.
  win::BitmapInfo bi;
  return bi.to_image(output_img);
}

bool lock::impl::get_image_spec(image_spec& spec) const {
  UINT cbformat;
  if (auto read_img = win::wic_image_format_available(&cbformat)) {
    HANDLE handle = GetClipboardData(cbformat);
    if (auto hlock = HglobalLock(handle)) {
      size_t size = GlobalSize(handle);
      uint8_t* data = hlock.data<uint8_t*>();
      bool result = read_img(data, size, nullptr, &spec);
      if (result)
        return true;
    }
  }

  win::BitmapInfo bi;
  if (!bi.is_valid())
    return false;
  bi.fill_spec(spec);
  return true;
}

#endif // CLIP_ENABLE_IMAGE

format register_format(const std::string& name) {
  int reqsize = 1+MultiByteToWideChar(CP_UTF8, 0,
                                      name.c_str(), name.size(), NULL, 0);
  std::vector<WCHAR> buf(reqsize);
  MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(),
                      &buf[0], reqsize);

  // From MSDN, registered clipboard formats are identified by values
  // in the range 0xC000 through 0xFFFF.
  return (format)RegisterClipboardFormatW(&buf[0]);
}

} // namespace clip
