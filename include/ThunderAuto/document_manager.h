#pragma once

#include <ThunderAuto/history_manager.h>
#include <ThunderAuto/project_state.h>
#include <ThunderAuto/thunder_auto.h>

class DocumentManager {
  ProjectSettings m_settings;
  HistoryManager m_history;

  bool m_open = false;
  std::string m_name;

public:
  DocumentManager() = default;
  ~DocumentManager() = default;

  constexpr const ProjectSettings& settings() const { return m_settings; }

  constexpr HistoryManager* history() { return &m_history; }
  constexpr const HistoryManager* history() const { return &m_history; }

  constexpr bool is_open() const { return m_open; }

  constexpr bool is_unsaved() const {
    if (!m_open) {
      return false;
    }

    return m_history.is_unsaved();
  }

  constexpr const std::filesystem::path& path() const {
    return m_settings.path;
  }
  constexpr std::string_view name() const { return m_name; }

  void new_project(ProjectSettings settings);
  void open_project(std::filesystem::path path);

  void save();

  inline void save_as(std::filesystem::path path) {
    m_settings.path = path;
    m_name = m_settings.path.filename().string();
    save();
  }

  void close();

  inline void undo() { m_history.undo(); }
  inline void redo() { m_history.redo(); }
};
