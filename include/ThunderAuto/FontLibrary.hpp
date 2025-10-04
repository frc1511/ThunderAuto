#pragma once

#include <imgui.h>

struct FontLibrary {
  ImFont* regularFont = nullptr;
  ImFont* boldFont = nullptr;
  ImFont* bigFont = nullptr;

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
