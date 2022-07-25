#include <project.h>
#include <fstream>
#include <cmath>

#include <pages/path_editor.h>
#include <pages/path_manager.h>

ProjectManager::ProjectManager() {

}

ProjectManager::~ProjectManager() {

}

void ProjectManager::new_project(ProjectSettings settings) {
  project.settings = settings;
  project.points = PathEditorPage::CurvePointTable({
    { 0.9f, 0.5f, -M_PI_2, 0.3f, 0.3f, 0.0f },
    { 0.5f, 0.3f, +M_PI_2, 0.3f, 0.3f, 0.0f },
    { 0.1f, 0.4f, +M_PI_2, 0.3f, 0.3f, 0.0f },
  });

  working_project = true;
  unsaved = false;

  PathEditorPage::get()->set_project(&project);
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
