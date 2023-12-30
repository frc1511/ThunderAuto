#pragma once

#include <ThunderAuto/platform/platform_manager.h>
#include <ThunderAuto/popups/new_field_popup.h>
#include <ThunderAuto/popups/popup.h>
#include <ThunderAuto/project_settings.h>
#include <ThunderAuto/thunder_auto.h>

class NewProjectPopup : public Popup {
  PlatformManager& m_platform_manager;

  const char* m_name = "New Project";

  ProjectSettings m_project;

  std::optional<Field> m_field = std::nullopt;

public:
  inline NewProjectPopup(PlatformManager& platform_manager)
    : m_platform_manager(platform_manager) {}

  void present(bool* running) override;
  constexpr const char* name() override { return m_name; }

  enum class Result {
    NONE,
    NEW_FIELD,
    CREATE,
    CANCEL,
  };

  constexpr Result result() const { return m_result; }

  inline void set_field(Field field) { m_field = field; }

  inline ProjectSettings result_project() const { return m_project; }

private:
  Result m_result = Result::NONE;
};
