#pragma once

#include <ThunderAuto/document_manager.h>
#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/thunder_auto.h>

class SettingsPage : public Page {
  DocumentManager& m_document_manager;

public:
  explicit SettingsPage(DocumentManager& document_manager)
    : m_document_manager(document_manager) {}

  constexpr const char* name() const override { return "Settings"; }

  void present(bool* running) override;
};

