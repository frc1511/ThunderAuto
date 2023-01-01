#include <project.h>
#include <pages/path_editor.h>
#include <pages/path_manager.h>
#include <pages/properties.h>
#include <pages/settings.h>

void to_json(nlohmann::json& json, const ProjectSettings& settings) {
  json = nlohmann::json {
    { "field", settings.field },
    { "drive_ctrl", static_cast<std::size_t>(settings.drive_ctrl) },
    { "max_accel", settings.max_accel },
    { "max_vel", settings.max_vel },
    { "robot_length", settings.robot_length },
    { "robot_width", settings.robot_width }
  };
}

void from_json(const nlohmann::json& json, ProjectSettings& settings) {
  settings.field = json.at("field").get<Field>();
  settings.drive_ctrl = static_cast<DriveController>(json.at("drive_ctrl").get<std::size_t>());
  settings.max_accel = json.at("max_accel").get<float>();
  settings.max_vel = json.at("max_vel").get<float>();
  settings.robot_length = json.at("robot_length").get<float>();
  settings.robot_width = json.at("robot_width").get<float>();
}

void to_json(nlohmann::json& json, const Project& project) {
  json = nlohmann::json {
    { "settings", project.settings },
    { "paths", project.paths }
  };
}

void from_json(const nlohmann::json& json, Project& project) {
  project.settings = json.at("settings").get<ProjectSettings>();
  project.paths = json.at("paths").get<std::vector<std::pair<std::string, PathEditorPage::CurvePointTable>>>();
}

ProjectManager::ProjectManager() { }

ProjectManager::~ProjectManager() { }

void ProjectManager::new_project(ProjectSettings _settings) {
  project.settings = _settings;
  project.paths.clear();
  project.paths.emplace_back("the_path", PathEditorPage::CurvePointTable({
    { 8.124f, 1.78f, 4.73853f, 4.73853f, 1.44372f, 1.70807f, 4.73853, false, true, false, 0 },
    { 4.0f,   1.5f,  2.0944f,  2.0944f,  2.0f,     2.0f,     2.0944,  false, false, true, 0 },
  }));

  working_project = true;
  unsaved = false;

  PathManagerPage::get()->set_project(&project);
  PathEditorPage::get()->set_project(&project);
  PropertiesPage::get()->set_project(&project);
  SettingsPage::get()->set_project(&project);

  save_project();
}

void ProjectManager::open_project(std::string path) {
  std::ifstream file(path);
  if (!file) return;

  nlohmann::json json;
  file >> json;
  project = json.get<Project>();
  project.settings.path = path;

  PathManagerPage::get()->set_project(&project);
  PathEditorPage::get()->set_project(&project);
  PropertiesPage::get()->set_project(&project);
  SettingsPage::get()->set_project(&project);
  working_project = true;
}

void ProjectManager::save_project() {
  std::ofstream file(project.settings.path);
  
  nlohmann::json json = project;
  file << json.dump(2);
}

void ProjectManager::save_project_as(std::string path) {
  project.settings.path = path;
  save_project();
}

void ProjectManager::close_project() {
  working_project = false;
}

ProjectManager ProjectManager::instance {};
