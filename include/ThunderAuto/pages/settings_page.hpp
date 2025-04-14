#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/document_manager.hpp>
#include <ThunderAuto/pages/page.hpp>

class SettingsPage : public Page {
  DocumentManager& m_document_manager;

public:
  explicit SettingsPage(DocumentManager& document_manager)
    : m_document_manager(document_manager) {}

  constexpr const char* name() const override { return "Settings"; }

  void present(bool* running) override;
};

