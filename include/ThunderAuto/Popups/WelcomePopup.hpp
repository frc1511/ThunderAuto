#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/FontLibrary.hpp>
#include <ThunderLibCore/RecentItemList.hpp>
#include <optional>
#include <filesystem>

using namespace thunder::core;

class WelcomePopup : public Popup {
  using RecentProjectList = RecentItemList<std::filesystem::path, 15>;

  RecentProjectList& m_recentProjects;
  std::optional<RecentProjectList::const_iterator> m_recentProjectIt;

 public:
  explicit WelcomePopup(RecentProjectList& recentProjects) : m_recentProjects(recentProjects) {}

  void present(bool* running) override;

  const char* name() const noexcept override { return "##Welcome!"; }

  enum class Result {
    NONE,
    NEW_PROJECT,
    OPEN_PROJECT,
    RECENT_PROJECT,
  };

  Result result() const { return m_result; }
  RecentProjectList::const_iterator recentProject() const {
    return m_recentProjectIt.value_or(m_recentProjects.end());
  }

 private:
  Result m_result = Result::NONE;
};
