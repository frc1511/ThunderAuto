#pragma once

#include <ThunderAuto/thunder_auto.h>

class Angle {
  float m_angle; // radians

private:
  inline Angle(float angle) { normalize_and_set(angle); }

  inline void normalize_and_set(float radians) {
    m_angle = std::fmod(radians, std::numbers::pi_v<float> * 2);
    if (m_angle < 0) {
      m_angle += std::numbers::pi_v<float> * 2;
    }
  }

public:
  static inline Angle radians(float radians) { return Angle(radians); }

  static inline Angle degrees(float degrees) {
    return Angle(degrees * std::numbers::pi_v<float> / 180.f);
  }

  inline float radians() const { return m_angle; }
  inline float degrees() const {
    return m_angle * 180 / std::numbers::pi_v<float>;
  }

  inline void set_radians(float radians) { normalize_and_set(radians); }
  inline void set_degrees(float degrees) {
    normalize_and_set(degrees * std::numbers::pi_v<float> / 180);
  }

  inline Angle operator+(Angle other) const {
    return Angle(m_angle + other.m_angle);
  }

  inline Angle operator-(Angle other) const {
    return Angle(m_angle - other.m_angle);
  }

  inline Angle operator*(float scalar) const { return Angle(m_angle * scalar); }

  inline Angle operator/(float scalar) const { return Angle(m_angle / scalar); }

  inline bool operator==(Angle other) const {
    return float_eq(m_angle, other.m_angle);
  }

  inline Angle supplementary() const {
    return Angle(m_angle + std::numbers::pi_v<float>);
  }

  inline bool is_supplementary(Angle other) const {
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
