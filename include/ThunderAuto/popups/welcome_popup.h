#pragma once

#include <ThunderAuto/font_library.h>
#include <ThunderAuto/popups/popup.h>

class WelcomePopup : public Popup {
  const char* m_name = "##Welcome!";

  FontLibrary& m_font_lib;

public:
  inline WelcomePopup(FontLibrary& font_lib)
    : m_font_lib(font_lib) {}

  void present(bool* running) override;

  constexpr const char* name() override { return m_name; }

  enum class Result {
    NONE,
    NEW_PROJECT,
    OPEN_PROJECT,
  };

  constexpr Result result() const { return m_result; }

private:
  Result m_result = Result::NONE;
};
