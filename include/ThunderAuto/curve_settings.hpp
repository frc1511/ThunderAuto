#pragma once

#include <ThunderAuto/thunder_auto.hpp>

struct CurveSettings {
  float max_linear_accel = 1.5f;           // m/s^2
  float max_linear_vel = 2.f;              // m/s
  float max_centripetal_accel = 2 * 9.81f; // m/s^2
};

void to_json(nlohmann::json& json, const CurveSettings& settings);
void from_json(const nlohmann::json& json, CurveSettings& settings);
