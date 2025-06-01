#include <ThunderAuto/curve_settings.hpp>

void to_json(wpi::json& json, const CurveSettings& settings) {
  json = wpi::json {
      {"max_linear_accel", settings.max_linear_accel},
      {"max_linear_vel", settings.max_linear_vel},
      {"max_centripetal_accel", settings.max_centripetal_accel}};
}

void from_json(const wpi::json& json, CurveSettings& settings) {
  settings.max_linear_accel = json.at("max_linear_accel").get<float>();
  settings.max_linear_vel = json.at("max_linear_vel").get<float>();
  settings.max_centripetal_accel =
      json.at("max_centripetal_accel").get<float>();
}
