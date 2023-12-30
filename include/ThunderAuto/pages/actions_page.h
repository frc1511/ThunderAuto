#pragma once

#include <ThunderAuto/curve.h>
#include <ThunderAuto/document_edit_manager.h>
#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/thunder_auto.h>

class ActionsPage : public Page {
  DocumentEditManager& m_history;

public:
  inline ActionsPage(DocumentEditManager& history)
    : m_history(history) {}

  constexpr const char* name() const override { return "Actions"; }

  void present(bool* running) override;
};
