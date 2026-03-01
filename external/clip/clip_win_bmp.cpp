// Clip Library
// Copyright (c) 2015-2025  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "clip_win_bmp.h"

#include "clip.h"
#include "clip_common.h"
#include "clip_win_hglobal.h"

#include <algorithm>
#include <vector>

namespace clip {
namespace win {

namespace {

unsigned long get_shift_from_mask(unsigned long mask) {
  unsigned long shift = 0;
  for (shift=0; shift<sizeof(unsigned long)*8; ++shift)
    if (mask & (1 << shift))
      return shift;
  return shift;
}

} // anonymous namespace

BitmapInfo::BitmapInfo() {
  // Use DIBV5 only for 32 bpp uncompressed bitmaps and when all
  // masks are valid.
  if (IsClipboardFormatAvailable(CF_DIBV5)) {
    b5 = (BITMAPV5HEADER*)GetClipboardData(CF_DIBV5);
    if (load_from(b5))
      return;
  }

  if (IsClipboardFormatAvailable(CF_DIB)) {
    bi = (BITMAPINFO*)GetClipboardData(CF_DIB);
    load_from(bi);
  }
}

void BitmapInfo::calc_stride() {
  // From:
  //
  //   https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader#calculating-surface-stride
  //
  // The stride (bytes per row) must be always the image width in
  // bytes rounded up to the nearest DWORD.
  //
  // Some programs (e.g. Steam client, in its screenshots section)
  // create an misaligned stride, this is just wrong. Not even MS
  // Paint is able to paste these images.
  stride = GDI_WIDTHBYTES(width * bit_count);
}

bool BitmapInfo::load_from(BITMAPV5HEADER* b5) {
  if (b5 &&
      b5->bV5BitCount == 32 &&
      ((b5->bV5Compression == BI_RGB) ||
       (b5->bV5Compression == BI_BITFIELDS &&
        b5->bV5RedMask && b5->bV5GreenMask &&
        b5->bV5BlueMask && b5->bV5AlphaMask))) {
    width       = b5->bV5Width;
    height      = b5->bV5Height;
    bit_count   = b5->bV5BitCount;
    compression = b5->bV5Compression;
    calc_stride();

    if (compression == BI_BITFIELDS) {
      red_mask    = b5->bV5RedMask;
      green_mask  = b5->bV5GreenMask;
      blue_mask   = b5->bV5BlueMask;
      alpha_mask  = b5->bV5AlphaMask;
    }
    else {
      red_mask    = 0xff0000;
      green_mask  = 0xff00;
      blue_mask   = 0xff;
      alpha_mask  = 0xff000000;
    }
    return true;
  }
  return false;
}

bool BitmapInfo::load_from(BITMAPINFO* bi) {
  if (!bi)
    return false;

  width       = bi->bmiHeader.biWidth;
  height      = bi->bmiHeader.biHeight;
  bit_count   = bi->bmiHeader.biBitCount;
  compression = bi->bmiHeader.biCompression;
  calc_stride();

  if (compression == BI_BITFIELDS) {
    red_mask   = *((uint32_t*)&bi->bmiColors[0]);
    green_mask = *((uint32_t*)&bi->bmiColors[1]);
    blue_mask  = *((uint32_t*)&bi->bmiColors[2]);
    if (bit_count == 32)
      alpha_mask = 0xff000000;
    return true;
  }
  if (compression == BI_RGB) {
    switch (bit_count) {
      case 32:
        red_mask   = 0xff0000;
        green_mask = 0xff00;
        blue_mask  = 0xff;
        alpha_mask = 0xff000000;
        break;
      case 24:
      case 8: // We return 8bpp, 4bpp and 1bpp images as 24bpp
      case 4:
      case 1:
        red_mask   = 0xff0000;
        green_mask = 0xff00;
        blue_mask  = 0xff;
        break;
      case 16:
        red_mask   = 0x7c00;
        green_mask = 0x03e0;
        blue_mask  = 0x001f;
        break;
    }
    return true;
  }
  return false;
}

BitmapInfo::BitmapInfo(BITMAPV5HEADER* pb5) {
  if (load_from(pb5))
    b5 = pb5;
}

BitmapInfo::BitmapInfo(BITMAPINFO* pbi) {
  if (load_from(pbi))
    bi = pbi;
}

void BitmapInfo::fill_spec(image_spec& spec) const {
  spec.width = width;
  spec.height = (height >= 0 ? height: -height);
  // We convert indexed to 24bpp RGB images to match the OS X behavior
  spec.bits_per_pixel = bit_count;
  if (spec.bits_per_pixel <= 8)
    spec.bits_per_pixel = 24;
  spec.bytes_per_row = width * ((spec.bits_per_pixel+7)/8);
  spec.red_mask   = red_mask;
  spec.green_mask = green_mask;
  spec.blue_mask  = blue_mask;
  spec.alpha_mask = alpha_mask;

  switch (spec.bits_per_pixel) {

    case 24: {
      // We need one extra byte to avoid a crash updating the last
      // pixel on last row using:
      //
      //   *((uint32_t*)ptr) = pixel24bpp;
      //
      ++spec.bytes_per_row;

      // Align each row to 32bpp
      int padding = (4-(spec.bytes_per_row&3))&3;
      spec.bytes_per_row += padding;
      break;
    }

    case 16: {
      int padding = (4-(spec.bytes_per_row&3))&3;
      spec.bytes_per_row += padding;
      break;
    }
  }
  spec.bytes_per_row = std::max<unsigned long>(spec.bytes_per_row, stride);

  unsigned long* masks = &spec.red_mask;
  unsigned long* shifts = &spec.red_shift;
  for (unsigned long* shift=shifts, *mask=masks; shift<shifts+4; ++shift, ++mask) {
    if (*mask)
      *shift = get_shift_from_mask(*mask);
  }
}

bool BitmapInfo::to_image(image& output_img) const {
  if (!is_valid()) {
    // There is no valid image. Maybe because there is no image at all
    // in the clipboard when using the BitmapInfo default
    // constructor. No need to report this as an error, just return
    // false.
    return false;
  }

  image_spec spec;
  fill_spec(spec);
  image img(spec);

  int direction = -1;
  int topY = spec.height - 1;
  // If the DIB is a top-down bitmap, then we must reverse the writing.
  if ((b5 && b5->bV5Height < 0) ||
      (bi && bi->bmiHeader.biHeight < 0)) {
    topY = 0;
    direction = 1;
  }

  switch (bit_count) {

    case 32:
    case 24:
    case 16: {
      const uint8_t* src = nullptr;

      if (compression == BI_RGB ||
          compression == BI_BITFIELDS) {
        if (b5)
          src = ((uint8_t*)b5) + b5->bV5Size;
        else
          src = ((uint8_t*)bi) + bi->bmiHeader.biSize;
        if (compression == BI_BITFIELDS)
          src += sizeof(RGBQUAD)*3;
      }

      if (src) {
        for (long y = 0; y < spec.height; ++y, src += stride) {
          char* dst = img.data() + (topY + direction * y) * spec.bytes_per_row;
          std::copy(src, src + stride, dst);
        }
      }

      // Windows uses premultiplied RGB values, and we use straight
      // alpha. So we have to divide all RGB values by its alpha.
      if (bit_count == 32 && spec.alpha_mask) {
        details::divide_rgb_by_alpha(img);
      }
      break;
    }

    case 8: {
      assert(bi);

      const int colors = (bi->bmiHeader.biClrUsed > 0 ? bi->bmiHeader.biClrUsed: 256);
      std::vector<uint32_t> palette(colors);
      for (int c=0; c<colors; ++c) {
        palette[c] =
          (bi->bmiColors[c].rgbRed   << spec.red_shift) |
          (bi->bmiColors[c].rgbGreen << spec.green_shift) |
          (bi->bmiColors[c].rgbBlue  << spec.blue_shift);
      }

      const uint8_t* src = (((uint8_t*)bi) + bi->bmiHeader.biSize + sizeof(RGBQUAD)*colors);
      const uint8_t* srcY = src;

      for (long y = 0; y < spec.height; ++y, srcY += stride, src = srcY) {
        char* dst = img.data() + (topY + direction * y) * spec.bytes_per_row;

        for (unsigned long x=0; x<spec.width; ++x, ++src, dst+=3) {
          int idx = *src;
          if (idx < 0)
            idx = 0;
          else if (idx >= colors)
            idx = colors-1;

          *((uint32_t*)dst) = palette[idx];
        }
      }
      break;
    }

    case 4: {
      assert(bi);

      const int colors = 16;
      std::vector<uint32_t> palette(colors);
      for (int c=0; c<colors; ++c) {
        palette[c] =
          (bi->bmiColors[c].rgbRed   << spec.red_shift) |
          (bi->bmiColors[c].rgbGreen << spec.green_shift) |
          (bi->bmiColors[c].rgbBlue  << spec.blue_shift);
      }

      const uint8_t* src = (((uint8_t*)bi) + bi->bmiHeader.biSize + sizeof(RGBQUAD)*colors);
      const uint8_t* srcY = src;

      for (long y = 0; y < spec.height; ++y, srcY += stride, src = srcY) {
        char* dst = img.data() + (topY + direction * y) * spec.bytes_per_row;

        for (unsigned long x=0; x<spec.width; ++x, dst+=3) {
          int idx = src[x / 2];
          if (x & 1)
            idx &= 0x0f;
          else
            idx >>= 4;

          if (idx < 0)
            idx = 0;
          else if (idx >= colors)
            idx = colors-1;

          *((uint32_t*)dst) = palette[idx];
        }
      }
      break;
    }

    case 1: {
      assert(bi);

      const int colors = 2;
      std::vector<uint32_t> palette(colors);
      for (int c=0; c<colors; ++c) {
        palette[c] =
          (bi->bmiColors[c].rgbRed   << spec.red_shift) |
          (bi->bmiColors[c].rgbGreen << spec.green_shift) |
          (bi->bmiColors[c].rgbBlue  << spec.blue_shift);
      }

      const uint8_t* src = (((uint8_t*)bi) + bi->bmiHeader.biSize + sizeof(RGBQUAD)*colors);
      const uint8_t* srcY = src;

      for (long y = 0; y < spec.height; ++y, srcY += stride, src = srcY) {
        char* dst = img.data() + (topY + direction * y) * spec.bytes_per_row;

        for (unsigned long x=0; x<spec.width; ++x, dst+=3) {
          const int idx = (src[x / 8] & (128 >> (x & 7)) ? 1 : 0);

          *((uint32_t*)dst) = palette[idx];
        }
      }
      break;
    }

  }

  std::swap(output_img, img);
  return true;
}

HGLOBAL create_dibv5(const image& image) {
  const image_spec& spec = image.spec();
  image_spec out_spec = spec;

  int palette_colors = 0;
  int padding = 0;
  switch (spec.bits_per_pixel) {
    case 24: padding = (4-((spec.width*3)&3))&3; break;
    case 16: padding = ((4-((spec.width*2)&3))&3)/2; break;
    case 8:  padding = (4-(spec.width&3))&3; break;
  }
  out_spec.bytes_per_row += padding;

  // Create the BITMAPV5HEADER structure
  Hglobal hmem(
      sizeof(BITMAPV5HEADER)
      + palette_colors*sizeof(RGBQUAD)
      + out_spec.bytes_per_row*out_spec.height);
  if (!hmem)
    return nullptr;

  out_spec.red_mask    = 0x00ff0000;
  out_spec.green_mask  = 0xff00;
  out_spec.blue_mask   = 0xff;
  out_spec.alpha_mask  = 0xff000000;
  out_spec.red_shift   = 16;
  out_spec.green_shift = 8;
  out_spec.blue_shift  = 0;
  out_spec.alpha_shift = 24;

  HglobalLock hlock(hmem);
  if (!hlock)
    return nullptr;

  BITMAPV5HEADER* bi = hlock.data<BITMAPV5HEADER*>();
  bi->bV5Size = sizeof(BITMAPV5HEADER);
  bi->bV5Width = out_spec.width;
  bi->bV5Height = out_spec.height;
  bi->bV5Planes = 1;
  bi->bV5BitCount = (WORD)out_spec.bits_per_pixel;
  bi->bV5Compression = BI_RGB;
  bi->bV5SizeImage = out_spec.bytes_per_row*spec.height;
  bi->bV5RedMask   = out_spec.red_mask;
  bi->bV5GreenMask = out_spec.green_mask;
  bi->bV5BlueMask  = out_spec.blue_mask;
  bi->bV5AlphaMask = out_spec.alpha_mask;
  bi->bV5CSType = LCS_WINDOWS_COLOR_SPACE;
  bi->bV5Intent = LCS_GM_GRAPHICS;
  bi->bV5ClrUsed = 0;

  switch (spec.bits_per_pixel) {
    case 32: {
      const char* src = image.data();
      char* dst = (((char*)bi)+bi->bV5Size) + (out_spec.height-1)*out_spec.bytes_per_row;
      for (long y=spec.height-1; y>=0; --y) {
        const uint32_t* src_x = (const uint32_t*)src;
        uint32_t* dst_x = (uint32_t*)dst;

        for (unsigned long x=0; x<spec.width; ++x, ++src_x, ++dst_x) {
          uint32_t c = *src_x;
          int r = ((c & spec.red_mask  ) >> spec.red_shift  );
          int g = ((c & spec.green_mask) >> spec.green_shift);
          int b = ((c & spec.blue_mask ) >> spec.blue_shift );
          int a = ((c & spec.alpha_mask) >> spec.alpha_shift);

          // Windows requires premultiplied RGBA values
          r = r * a / 255;
          g = g * a / 255;
          b = b * a / 255;

          *dst_x =
            (r << out_spec.red_shift  ) |
            (g << out_spec.green_shift) |
            (b << out_spec.blue_shift ) |
            (a << out_spec.alpha_shift);
        }

        src += spec.bytes_per_row;
        dst -= out_spec.bytes_per_row;
      }
      break;
    }
    default:
      error_handler e = get_error_handler();
      if (e)
        e(ErrorCode::ImageNotSupported);
      return nullptr;
  }

  return hmem.release();
}

} // namespace win
} // namespace clip
