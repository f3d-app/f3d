# Clip Library
*Copyright (c) 2015-2025 David Capello*

[![build](https://github.com/dacap/clip/workflows/build/badge.svg)](https://github.com/dacap/clip/actions?query=workflow%3Abuild)
[![MIT Licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.txt)

Library to copy/retrieve content to/from the clipboard/pasteboard.

## Features

Available features on Windows, macOS, and Linux (X11):

* Copy/paste UTF-8 text.
* Copy/paste user-defined data.
* Copy/paste RGB/RGBA images. This library use non-premultiplied alpha RGB values.

## Example

```cpp
#include "clip.h"
#include <iostream>

int main() {
  clip::set_text("Hello World");

  std::string value;
  clip::get_text(value);
  std::cout << value << "\n";
}
```

## User-defined clipboard formats

```cpp
#include "clip.h"

int main() {
  clip::format my_format =
    clip::register_format("com.appname.FormatName");

  int value = 32;
  std::string str = "Alternative text for value 32";

  clip::lock l;
  l.clear();
  l.set_data(clip::text_format(), str.c_str(), str.size());
  l.set_data(my_format, (const char*)&value, sizeof(int));
}
```

## Platform specific details

* If two versions of your application (32-bit and 64-bit) can run at
  at the same time, remember to avoid storing data types that could
  change depending on the platform (e.g. `size_t`) in your custom
  format data.
* **Windows**:
  - [Limited number of clipboard formats on Windows](https://web.archive.org/web/20250126161802/https://devblogs.microsoft.com/oldnewthing/20080430-00/?p=22523)
* **Linux**:
  - To be able to copy/paste on Linux you need `libx11-dev`/`libX11-devel` package.
  - To copy/paste images you will need `libpng-dev`/`libpng-devel` package.

## Compilation Flags

* `CLIP_ENABLE_IMAGE`: Enables the support to
  [copy](examples/put_image.cpp)/[paste](examples/show_image.cpp) images.
* `CLIP_ENABLE_LIST_FORMATS` (only for Windows): Enables the
  `clip::lock::list_formats()` API function and the
  [list_clip_formats](examples/list_clip_formats.cpp) example.
* `CLIP_EXAMPLES`: Compile [examples](examples/).
* `CLIP_TESTS`: Compile [tests](tests/).
* `CLIP_INSTALL`: Generate installation rules for CMake.
* `CLIP_X11_WITH_PNG` (only for Linux/X11): Enables support to
  copy/paste images using the `libpng` library on Linux.

## Who is using this library?

[Check the wiki](https://github.com/dacap/clip/wiki#who-is-using-clip)
to know what projects are using the `clip` library.
