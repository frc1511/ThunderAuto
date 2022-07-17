#include <project.h>
#include <fstream>

ProjectManager::ProjectManager() {

}

ProjectManager::~ProjectManager() {

}

void ProjectManager::new_project(ProjectSettings _settings) {
  settings = _settings;
  working_project = true;
  unsaved = false;
}

void ProjectManager::open_project(std::string path) {
  std::ifstream file(path);
  if (!file) return;

  // TODO: Parse file and load project settings.
}

void ProjectManager::save_project() {

}

void ProjectManager::save_project_as(std::string path) {

}

void ProjectManager::close_project() {
  working_project = false;
}

ProjectManager ProjectManager::instance {};
