#include <ThunderAuto/curve_settings.h>

void to_json(nlohmann::json& json, const CurveSettings& settings) {
  json = nlohmann::json {
      {"max_linear_accel", settings.max_linear_accel},
      {"max_linear_vel", settings.max_linear_vel},
      {"max_centripetal_accel", settings.max_centripetal_accel}};
}

void from_json(const nlohmann::json& json, CurveSettings& settings) {
  settings.max_linear_accel = json.at("max_linear_accel").get<float>();
  settings.max_linear_vel = json.at("max_linear_vel").get<float>();
  settings.max_centripetal_accel =
      json.at("max_centripetal_accel").get<float>();
}
