#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/field.hpp>

enum class DriveController {
  RAMSETE,
  HOLONOMIC,
};

struct ProjectSettings {
  std::filesystem::path path;

  // Default to the version before the version number was added to the settings.
  int version_major = 2025;
  int version_minor = 3;
  int version_patch = 0;

  Field field;

  DriveController drive_controller = DriveController::HOLONOMIC;

  float robot_length = 0.8; // m
  float robot_width = 0.8;  // m

  bool auto_save = false;
  bool auto_export = false;
};

void to_json(nlohmann::json& json, const ProjectSettings& settings);
void from_json(const nlohmann::json& json, ProjectSettings& settings);

