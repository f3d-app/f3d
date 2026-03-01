// Clip Library
// Copyright (c) 2025 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef CLIP_WIN_HGLOBAL_H_INCLUDED
#define CLIP_WIN_HGLOBAL_H_INCLUDED
#pragma once

#include <windows.h>

#include <algorithm>

namespace clip {
namespace win {

// Scoped GlobalLock / GlobalUnlock wrapper
class HglobalLock {
public:
  HglobalLock() = delete;
  HglobalLock(const HglobalLock& other) = delete;
  HglobalLock& operator=(const HglobalLock& other) = delete;
  HglobalLock& operator=(HglobalLock&& other) = delete;

  HglobalLock(HglobalLock&& other) {
    std::swap(m_handle, other.m_handle);
    std::swap(m_ptr, other.m_ptr);
  }

  explicit HglobalLock(HGLOBAL handle) : m_handle(handle) {
    if (m_handle)
      m_ptr = GlobalLock(m_handle);
  }

  ~HglobalLock() {
    if (m_handle)
      GlobalUnlock(m_handle);
  }

  template<typename T>
  T data() {
    return static_cast<T>(m_ptr);
  }

  operator bool() const {
    return m_ptr != nullptr;
  }

private:
  HGLOBAL m_handle = nullptr;
  LPVOID m_ptr = nullptr;
};

// Scoped GlobalAlloc / GlobalFree wrapper
class Hglobal {
public:
  Hglobal() : m_handle(nullptr) {
  }

  explicit Hglobal(HGLOBAL handle) : m_handle(handle) {
  }

  explicit Hglobal(size_t len) : m_handle(GlobalAlloc(GHND, len)) {
  }

  ~Hglobal() {
    if (m_handle)
      GlobalFree(m_handle);
  }

  HglobalLock lock() {
    return HglobalLock(m_handle);
  }

  HGLOBAL release() {
    HGLOBAL handle = m_handle;
    m_handle = nullptr;
    return handle;
  }

  operator HGLOBAL() {
    return m_handle;
  }

  // Sets the HGLOBAL handle as clipboard data, releasing the handle
  // (as now the handle is owned by the system) if the data can be
  // moved to the clipboard.
  bool set_clipboard_data(const UINT format) {
    bool result = (SetClipboardData(format, m_handle) ? true: false);
    if (result)
      release();
    return result;
  }

private:
  HGLOBAL m_handle;
};

} // namespace win
} // namespace clip

#endif // CLIP_WIN_HGLOBAL_H_INCLUDED
