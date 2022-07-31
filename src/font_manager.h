#pragma once

#include <thunder_auto.h>

struct FontManager {
  static FontManager* get() {
    return &instance;
  }

  ImFont* regular;
  ImFont* big;

private:
  static FontManager instance;
};
