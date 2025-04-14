#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/field.hpp>
#include <ThunderAuto/platform/platform_manager.hpp>
#include <ThunderAuto/popups/popup.hpp>
#include <ThunderAuto/texture.hpp>

class NewFieldPopup : public Popup {
  PlatformManager& m_platform_manager;

  const char* m_name = "New Field";

  bool m_selected_image = false;

  Texture m_field_texture;
  float m_field_aspect_ratio = 1;
  bool m_image_load_failed = false;

  std::optional<Field> m_field = std::nullopt;

  ImVec2 m_field_size = {16.54175f, 8.0137f}; // meters.

public:
  inline explicit NewFieldPopup(PlatformManager& platform_manager)
    : m_platform_manager(platform_manager) {}

  void present(bool* running) override;
  constexpr const char* name() override { return m_name; }

  inline const Field& field() const {
    assert(m_field);
    return *m_field;
  }

  enum class Result {
    NONE,
    CREATE,
    CANCEL,
  };

  constexpr Result result() const { return m_result; }

private:
  void present_field_setup();

private:
  Result m_result = Result::NONE;
};

