#pragma once

#include <ThunderAuto/popups/popup.h>

class UnsavedPopup : public Popup {
  const char* m_name = "Save Changes?";

public:
  void present(bool* running) override;
  constexpr const char* name() override { return m_name; }

  enum class Result {
    NONE,
    SAVE,
    DONT_SAVE,
    CANCEL,
  };

  constexpr Result result() const { return m_result; }

private:
  Result m_result = Result::NONE;
};
