// Clip Library
// Copyright (C) 2026 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef CLIP_BASE_H_INCLUDED
#define CLIP_BASE_H_INCLUDED
#pragma once

#if defined(_WIN32) && defined(CLIP_SHARED)
  #ifdef CLIP_BUILDING_LIB
    #define CLIP_EXTERN __declspec(dllexport)
  #else
    #define CLIP_EXTERN __declspec(dllimport)
  #endif
#else
  #define CLIP_EXTERN
#endif

#endif // CLIP_H_INCLUDED
