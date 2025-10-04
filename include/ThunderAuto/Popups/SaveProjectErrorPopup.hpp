#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <string>

class SaveProjectErrorPopup : public Popup {
  std::string m_error;

 public:
  SaveProjectErrorPopup() = default;

  void present(bool* running) override;
  const char* name() const noexcept override { return "Project Save Error"; }

  void setError(const std::string& error) { m_error = error; }
};
