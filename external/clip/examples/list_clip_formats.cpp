// Clip Library
// Copyright (c) 2024 David Capello

#include "clip.h"
#include <iostream>

int main() {
  clip::lock l;
  for (const clip::format_info& info : l.list_formats()) {
    std::cout << "- [" << info.id << "] " << info.name << "\n";
  }
}
