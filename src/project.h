#pragma once

#include <field.h>
#include <string>
#include <pages/path_editor.h>

enum class DriveController {
  RAMSETE,
  HOLONOMIC,
};

struct ProjectSettings {
  std::string path;
  
  Field field;
  
  DriveController drive_controller;
  
  double max_acceleration; // m/s2
  double max_deceleration; // m/s2
  
  double max_velocity; // m/s
};

struct Project {
  ProjectSettings settings;
  PathEditorPage::CurvePointTable points;
};

class ProjectManager {
public:
  static ProjectManager* get() {
    return &instance;
  }

  ProjectManager(ProjectManager const&) = delete;
  ProjectManager& operator=(ProjectManager const&) = delete;

  void new_project(ProjectSettings settings);
  void open_project(std::string path);

  void save_project();
  void save_project_as(std::string path);

  void close_project();

  constexpr bool has_project() const { return working_project; }

  constexpr bool is_unsaved() const { return working_project ? unsaved : false; }

private:
  ProjectManager();
  ~ProjectManager();
  
  Project project;

  bool working_project = false;
  bool unsaved = false;
  
  static ProjectManager instance;
};
