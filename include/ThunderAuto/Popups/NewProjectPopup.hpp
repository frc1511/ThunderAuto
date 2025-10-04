#pragma once

#include <ThunderAuto/Platform/PlatformManager.hpp>
#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>
#include <ThunderLibCore/Auto/ThunderAutoFieldImage.hpp>
#include <optional>
#include <filesystem>

using namespace thunder::core;

class NewProjectPopup : public Popup {
  PlatformManager& m_platformManager;

  // Result
  ThunderAutoProjectSettings m_project;

  // Working variables
  std::optional<ThunderAutoFieldImage> m_field = std::nullopt;

  std::filesystem::path m_projectPath;
  char m_projectPathBuf[256];
  size_t m_selectedFieldOptionIndex;
  size_t m_selectedControllerOptionIndex;
  float m_robotLength;
  float m_robotWidth;

 public:
  explicit NewProjectPopup(PlatformManager& platformManager) : m_platformManager(platformManager) { reset(); }

  void present(bool* running) override;
  const char* name() const noexcept override { return "New Project"; }

  void reset() {
    m_result = Result::NONE;
    m_field = std::nullopt;
    m_projectPath.clear();
    m_projectPathBuf[0] = '\0';
    m_selectedFieldOptionIndex = 0;
    m_selectedControllerOptionIndex = 0;
    m_robotLength = 0.8f;
    m_robotWidth = 0.8f;
  }

  enum class Result {
    NONE,
    NEW_FIELD,
    CREATE,
    CANCEL,
  };

  Result result() const { return m_result; }

  void setField(const ThunderAutoFieldImage& field) { m_field = field; }

  ThunderAutoProjectSettings resultProject() const { return m_project; }

 private:
  Result m_result = Result::NONE;
};
