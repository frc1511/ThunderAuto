#pragma once

#include <ThunderAuto/field.h>
#include <ThunderAuto/thunder_auto.h>

enum class DriveController {
  RAMSETE,
  HOLONOMIC,
};

struct ProjectSettings {
  std::filesystem::path path;

  int version_major = TH_VERSION_MAJOR;
  int version_minor = TH_VERSION_MINOR;
  int version_patch = TH_VERSION_PATCH;

  Field field;

  DriveController drive_controller = DriveController::HOLONOMIC;

  float robot_length = 0.8; // m
  float robot_width = 0.8;  // m

  bool auto_save = false;
  bool auto_export = false;
};

void to_json(nlohmann::json& json, const ProjectSettings& settings);
void from_json(const nlohmann::json& json, ProjectSettings& settings);

