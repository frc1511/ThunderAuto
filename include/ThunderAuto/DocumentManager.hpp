#pragma once

#include <ThunderAuto/HistoryManager.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>

using namespace thunder::core;

class DocumentManager final {
  ThunderAutoProjectSettings m_settings;
  HistoryManager m_history;

  bool m_open = false;

 public:
  const ThunderAutoProjectSettings& settings() const noexcept { return m_settings; }
  ThunderAutoProjectSettings& settings() noexcept { return m_settings; }

  const HistoryManager& history() const noexcept { return m_history; }
  HistoryManager& history() noexcept { return m_history; }

  bool isOpen() const noexcept { return m_open; }

  bool isUnsaved() const noexcept {
    if (!m_open)
      return false;

    return m_history.isUnsaved();
  }

  const std::filesystem::path& path() const noexcept { return m_settings.projectPath; }
  const std::string& name() const noexcept { return m_settings.name; }

  void newProject(ThunderAutoProjectSettings settings) noexcept;
  ThunderAutoProjectVersion openProject(const std::filesystem::path& path);

  void save();

  void setProjectPath(const std::filesystem::path& path) noexcept { m_settings.setProjectPath(path); }

  void close() noexcept;

  void undo() noexcept { m_history.undo(); }
  void redo() noexcept { m_history.redo(); }
};
