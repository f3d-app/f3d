// Clip Library
// Copyright (c) 2024 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef CLIP_WIN_H_INCLUDED
#define CLIP_WIN_H_INCLUDED
#pragma once

#include <windows.h>

#ifndef LCS_WINDOWS_COLOR_SPACE
#define LCS_WINDOWS_COLOR_SPACE 'Win '
#endif

#ifndef CF_DIBV5
#define CF_DIBV5                17
#endif

#if CLIP_ENABLE_IMAGE
  #include "clip_win_bmp.h"
  #include "clip_win_wic.h"
#endif

#endif // CLIP_WIN_H_INCLUDED
