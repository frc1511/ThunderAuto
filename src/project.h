#pragma once

#include <string>

enum class DriveController {
  RAMSETE,
  HOLONOMIC,
};

struct ProjectSettings {
  std::string path;
  
  DriveController drive_controller;
  
  double max_acceleration; // m/s2
  double max_deceleration; // m/s2
  
  double max_velocity; // m/s
};
