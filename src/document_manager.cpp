#include <ThunderAuto/document_manager.h>

void DocumentManager::new_project(ProjectSettings settings) {
  if (m_open) close();

  puts("New project");

  m_settings = settings;
  m_name = std::filesystem::path(m_settings.path).filename().string();
  m_open = true;

  m_history.reset(ProjectState {}, true);
}

OpenProjectStatus DocumentManager::open_project(std::filesystem::path path) {
  if (m_open) close();

  puts("Open project");

  using enum OpenProjectStatus;

  m_settings.path = path;

  if (path.empty() || !std::filesystem::exists(path)) return FILE_NOT_FOUND;

  std::ifstream file(path);
  if (!file.is_open()) return FAILED_TO_OPEN;

  ProjectState state;

  try {
    nlohmann::json json;
    file >> json;

    m_settings = json.at("settings").get<ProjectSettings>();
    m_settings.path = path;

    if (m_settings.version_major > TH_VERSION_MAJOR) return VERSION_TOO_NEW;

    m_name = path.filename().string();
    state = json.at("state").get<ProjectState>();

  } catch (...) {
    return INVALID_CONTENTS;
  }

  m_open = true;

  m_history.reset(state);

  return OK;
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

