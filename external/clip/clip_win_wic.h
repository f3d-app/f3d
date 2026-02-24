// Clip Library
// Copyright (c) 2020-2024 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef CLIP_WIN_WIC_H_INCLUDED
#define CLIP_WIN_WIC_H_INCLUDED
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

typedef bool (*ReadWicImageFormatFunc)(const uint8_t*,
                                       const UINT,
                                       clip::image*,
                                       clip::image_spec*);

ReadWicImageFormatFunc wic_image_format_available(UINT* output_cbformat);

//////////////////////////////////////////////////////////////////////
// Encode the image as PNG format

bool write_png_on_stream(const image& image, IStream* stream);

HGLOBAL write_png(const image& image);

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from PNG format

bool read_png(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec);

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from JPEG format

bool read_jpg(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec);

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from GIF format

bool read_gif(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec);

//////////////////////////////////////////////////////////////////////
// Decode the clipboard data from BMP format

bool read_bmp(const uint8_t* buf,
              const UINT len,
              image* output_image,
              image_spec* output_spec);


} // namespace win
} // namespace clip

#endif // CLIP_WIN_WIC_H_INCLUDED
