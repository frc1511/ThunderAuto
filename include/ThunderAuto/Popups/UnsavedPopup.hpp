#pragma once

#include <ThunderAuto/Popups/Popup.hpp>

class UnsavedPopup : public Popup {
 public:
  const char* name() const noexcept override { return "Save Changes?"; }

  void present(bool* running) override;

  enum class Result {
    NONE,
    SAVE,
    DONT_SAVE,
    CANCEL,
  };

  Result result() const { return m_result; }

 private:
  Result m_result = Result::NONE;
};
