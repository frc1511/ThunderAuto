#include <ThunderAuto/angle.hpp>

Angle::Angle(float angle, Bounds bounds) {
  m_bounds = bounds;
  normalize_and_set(angle);
}

void Angle::normalize_and_set(float radians) {
  if (!std::isfinite(radians)) {
    m_angle = 0;
    return;
  }

  // There's probably a better way...

  float lower, upper;
  if (m_bounds == Bounds::ZERO_TO_2PI) {
    lower = 0;
    upper = std::numbers::pi_v<float> * 2;
  } else {
    lower = -std::numbers::pi_v<float>;
    upper = std::numbers::pi_v<float>;
  }

  while (radians < lower) {
    radians += std::numbers::pi_v<float> * 2;
  }

  while (radians >= upper) {
    radians -= std::numbers::pi_v<float> * 2;
  }

  m_angle = radians;
}

