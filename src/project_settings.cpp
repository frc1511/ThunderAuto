#include <ThunderAuto/project_settings.hpp>

void to_json(nlohmann::json& json, const ProjectSettings& settings) {
  json = nlohmann::json {
      {"version_major", THUNDER_AUTO_VERSION_MAJOR},
      {"version_minor", THUNDER_AUTO_VERSION_MINOR},
      {"version_patch", THUNDER_AUTO_VERSION_PATCH},
      {"field", settings.field},
      {"drive_ctrl", static_cast<std::size_t>(settings.drive_controller)},
      {"robot_length", settings.robot_length},
      {"robot_width", settings.robot_width},
      {"auto_save", settings.auto_save},
      {"auto_export", settings.auto_export},
  };
}

void from_json(const nlohmann::json& json, ProjectSettings& settings) {
  if (json.contains("version_major")) {
    settings.version_major = json.at("version_major").get<int>();
  }
  if (json.contains("version_minor")) {
    settings.version_minor = json.at("version_minor").get<int>();
  }
  if (json.contains("version_patch")) {
    settings.version_patch = json.at("version_patch").get<int>();
  }

  settings.field = json.at("field").get<Field>();
  settings.drive_controller =
      static_cast<DriveController>(json.at("drive_ctrl").get<std::size_t>());
  settings.robot_length = json.at("robot_length").get<float>();
  settings.robot_width = json.at("robot_width").get<float>();

  settings.auto_save = settings.auto_export = false;

  if (json.contains("auto_save")) {
    settings.auto_save = json.at("auto_save").get<bool>();
  }
  if (json.contains("auto_export")) {
    settings.auto_export = json.at("auto_export").get<bool>();
  }
}

