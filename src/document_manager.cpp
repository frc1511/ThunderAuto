#include <ThunderAuto/document_manager.h>

void DocumentManager::new_project(ProjectSettings settings) {
  if (m_open) close();

  puts("New project");

  m_settings = settings;
  m_name = std::filesystem::path(m_settings.path).filename().string();
  m_open = true;

  m_history.reset(ProjectState{}, true);
}

void DocumentManager::open_project(std::filesystem::path path) {
  if (m_open) close();

  puts("Open project");

  std::ifstream file(path);
  assert(file.is_open());

  nlohmann::json json;
  file >> json;

  m_settings = json.at("settings").get<ProjectSettings>();
  m_settings.path = path;
  m_name = path.filename().string();

  ProjectState state = json.at("state").get<ProjectState>();

  m_open = true;

  m_history.reset(state);
}

void DocumentManager::save() {
  if (!m_open) return;

  puts("Save project");

  nlohmann::json json = nlohmann::json {
      {"state", m_history.current_state()},
      {"settings", m_settings},
  };

  std::ofstream file(m_settings.path);
  if (!file.is_open()) {
    puts("Failed to open project file");
    return;
  }
  file << json.dump(2);

  m_history.mark_saved();
}

void DocumentManager::close() {
  puts("Close project");

  m_open = false;
  m_settings = {};
  m_name = "";
}
