#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <string>

struct Field {
  std::string img_path;
  ImVec2 min, max;
};
