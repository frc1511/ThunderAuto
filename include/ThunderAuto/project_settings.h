#pragma once

#include <ThunderAuto/field.h>
#include <ThunderAuto/thunder_auto.h>

enum class DriveController {
  RAMSETE,
  HOLONOMIC,
};

struct ProjectSettings {
  std::filesystem::path path;

  Field field;

  DriveController drive_controller;

  float robot_length; // m
  float robot_width;  // m
};

void to_json(nlohmann::json& json, const ProjectSettings& settings);
void from_json(const nlohmann::json& json, ProjectSettings& settings);
