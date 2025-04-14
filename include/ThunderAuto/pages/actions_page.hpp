#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/curve.hpp>
#include <ThunderAuto/document_edit_manager.hpp>
#include <ThunderAuto/pages/page.hpp>

class ActionsPage : public Page {
  DocumentEditManager& m_history;

public:
  inline ActionsPage(DocumentEditManager& history)
    : m_history(history) {}

  constexpr const char* name() const override { return "Actions"; }

  void present(bool* running) override;
};
