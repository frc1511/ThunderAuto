#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <string>

class OpenProjectErrorPopup : public Popup {
  std::string m_error;

 public:
  OpenProjectErrorPopup() = default;

  void present(bool* running) override;
  const char* name() const noexcept override { return "Project Open Error"; }

  void setError(const std::string& error) { m_error = error; }
};
