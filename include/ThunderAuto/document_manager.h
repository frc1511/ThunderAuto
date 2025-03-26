#pragma once

#include <ThunderAuto/history_manager.h>
#include <ThunderAuto/project_state.h>
#include <ThunderAuto/thunder_auto.h>

enum class OpenProjectStatus {
  OK,
  FILE_NOT_FOUND,
  FAILED_TO_OPEN,
  VERSION_TOO_NEW,
  INVALID_CONTENTS,
};

class DocumentManager {
  ProjectSettings m_settings;
  HistoryManager m_history;

  bool m_open = false;
  std::string m_name;

public:
  DocumentManager() = default;
  ~DocumentManager() = default;

  const ProjectSettings& settings() const { return m_settings; }
  ProjectSettings& settings() { return m_settings; }

  HistoryManager* history() { return &m_history; }
  const HistoryManager* history() const { return &m_history; }

  bool is_open() const { return m_open; }

  bool is_unsaved() const {
    if (!m_open) {
      return false;
    }

    return m_history.is_unsaved();
  }

  const std::filesystem::path& path() const {
    return m_settings.path;
  }
  const std::string& name() const { return m_name; }

  void new_project(ProjectSettings settings);
  OpenProjectStatus open_project(std::filesystem::path path);

  void save();

  void save_as(std::filesystem::path path) {
    m_settings.path = path;
    m_name = m_settings.path.filename().string();
    save();
  }

  void close();

  void undo() { m_history.undo(); }
  void redo() { m_history.redo(); }
};
