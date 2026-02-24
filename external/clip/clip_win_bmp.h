// Clip Library
// Copyright (c) 2015-2025  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef CLIP_WIN_BMP_H_INCLUDED
#define CLIP_WIN_BMP_H_INCLUDED
#pragma once

#if !CLIP_ENABLE_IMAGE
  #error This file can be include only when CLIP_ENABLE_IMAGE is defined
#endif

#include <cstdint>

#include <windows.h>

namespace clip {

class image;
struct image_spec;

namespace win {

struct BitmapInfo {
  BITMAPV5HEADER* b5 = nullptr;
  BITMAPINFO* bi = nullptr;
  int width = 0;
  int height = 0;
  uint16_t bit_count = 0;
  uint32_t compression = 0;
  uint32_t stride = 0;
  uint32_t red_mask = 0;
  uint32_t green_mask = 0;
  uint32_t blue_mask = 0;
  uint32_t alpha_mask = 0;

  BitmapInfo();
  explicit BitmapInfo(BITMAPV5HEADER* pb5);
  explicit BitmapInfo(BITMAPINFO* pbi);

  bool is_valid() const {
    return (b5 || bi);
  }

  void fill_spec(image_spec& spec) const;

  // Fills the output_img with the data provided by this
  // BitmapInfo. Returns true if it was able to fill the output image
  // or false otherwise.
  bool to_image(image& output_img) const;

private:
  void calc_stride();
  bool load_from(BITMAPV5HEADER* b5);
  bool load_from(BITMAPINFO* bi);
};

// Returns a handle to the HGLOBAL memory reserved to create a DIBV5
// based on the image passed by parameter. Returns null if it cannot
// create the handle.
HGLOBAL create_dibv5(const image& image);

} // namespace win
} // namespace clip

#endif // CLIP_WIN_BMP_H_INCLUDED
