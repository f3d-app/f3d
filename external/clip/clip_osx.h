// Clip Library
// Copyright (c) 2024 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef CLIP_OSX_H_INCLUDED
#define CLIP_OSX_H_INCLUDED
#pragma once

#ifdef __OBJC__

#include <Cocoa/Cocoa.h>

namespace clip {

class image;
struct image_spec;

namespace osx {

#if CLIP_ENABLE_IMAGE

bool get_image_from_clipboard(NSPasteboard* pasteboard,
                              image* output_img,
                              image_spec* output_spec);

#endif // CLIP_ENABLE_IMAGE

} // namespace osx
} // namespace clip

#endif

#endif
