#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <string_view>

class RenameActionPopup : public Popup {
  DocumentEditManager& m_history;

  char m_newActionNameBuffer[256] = "";
  std::string m_oldActionName;

 public:
  explicit RenameActionPopup(DocumentEditManager& history) : m_history(history) {}

  const char* name() const noexcept override { return "Rename Action"; }

  void present(bool* running) override;

  void setOldActionName(std::string_view name) {
    m_oldActionName = name;

    size_t copySize = std::min(name.size(), sizeof(m_newActionNameBuffer) - 1);
    std::memcpy(m_newActionNameBuffer, name.data(), copySize);
    m_newActionNameBuffer[copySize] = '\0';
  }

  void reset() noexcept { m_newActionNameBuffer[0] = '\0'; }
};
