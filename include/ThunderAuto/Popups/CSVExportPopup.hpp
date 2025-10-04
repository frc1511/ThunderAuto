#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <string>

class CSVExportPopup : public Popup {
  std::string m_exportMessage;

 public:
  CSVExportPopup() = default;

  void present(bool* running) override;
  const char* name() const noexcept override { return "CSV Export"; }

  void setExportMessage(const std::string& message) { m_exportMessage = message; }
};
