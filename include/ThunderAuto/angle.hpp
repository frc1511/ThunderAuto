#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/math_util.hpp>

/**
 * @brief Represents an angle. Handles conversion from radians to degrees and
 *        normalization of angles.
 */
class Angle {
  float m_angle;  // radians

 public:
  enum class Bounds {
    ZERO_TO_2PI,
    NEG_PI_TO_POS_PI,
    ZERO_TO_360 = ZERO_TO_2PI,
    NEG_180_TO_POS_180 = NEG_PI_TO_POS_PI,
  };

 private:
  Bounds m_bounds = Bounds::ZERO_TO_2PI;

 private:
  Angle(float angle, Bounds bounds);
  void normalize_and_set(float radians);

 public:
  Angle() : m_angle(0) {}

  static Angle radians(float radians, Bounds bounds = Bounds::ZERO_TO_2PI) {
    return Angle(radians, bounds);
  }

  static Angle degrees(float degrees, Bounds bounds = Bounds::ZERO_TO_360) {
    return Angle(degrees * std::numbers::pi_v<float> / 180.f, bounds);
  }

  float radians() const { return m_angle; }
  float degrees() const { return m_angle * 180.f / std::numbers::pi_v<float>; }

  void set_bounds(Bounds bounds) {
    m_bounds = bounds;
    normalize_and_set(m_angle);
  }

  void set_radians(float radians) { normalize_and_set(radians); }
  void set_degrees(float degrees) {
    normalize_and_set(degrees * std::numbers::pi_v<float> / 180.f);
  }

  Angle operator+(Angle other) const {
    return Angle(m_angle + other.m_angle, m_bounds);
  }

  Angle operator-(Angle other) const {
    return Angle(m_angle - other.m_angle, m_bounds);
  }

  Angle operator*(float scalar) const {
    return Angle(m_angle * scalar, m_bounds);
  }

  Angle operator/(float scalar) const {
    return Angle(m_angle / scalar, m_bounds);
  }

  bool operator==(Angle other) const {
    return float_eq(m_angle, other.m_angle);
  }

  Angle supplementary() const {
    return Angle(m_angle + std::numbers::pi_v<float>, m_bounds);
  }

  bool is_supplementary(Angle other) const {
    return *this == other + Angle::radians(std::numbers::pi_v<float>);
  }
};

inline Angle operator"" _deg(long double degrees) {
  return Angle::degrees(static_cast<float>(degrees));
}

inline Angle operator"" _deg(unsigned long long degrees) {
  return Angle::degrees(static_cast<float>(degrees));
}

inline Angle operator"" _rad(long double radians) {
  return Angle::degrees(static_cast<float>(radians));
}

inline Angle operator"" _rad(unsigned long long radians) {
  return Angle::degrees(static_cast<float>(radians));
}

