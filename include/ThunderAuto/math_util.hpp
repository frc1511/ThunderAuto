#pragma once

#include <ThunderAuto/thunder_auto.hpp>

inline bool float_eq(float x, float y) {
  return std::fabs(x - y) < std::numeric_limits<float>::epsilon();
}

