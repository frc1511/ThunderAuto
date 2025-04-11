#pragma once

#include <ThunderAuto/thunder_auto.h>

struct FontLibrary {
  ImFont* regular_font = nullptr;
  ImFont* big_font = nullptr;

private:
  FontLibrary() = default;

public:
  static FontLibrary& get() {
    static FontLibrary instance;
    return instance;
  }

  FontLibrary(FontLibrary const&) = delete;
  void operator=(FontLibrary const&) = delete;
};
