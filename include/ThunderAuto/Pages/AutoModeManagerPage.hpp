#pragma once

#include <ThunderAuto/Pages/Page.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <cstddef>

using namespace thunder::core;

class AutoModeManagerPage : public Page {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

 public:
  AutoModeManagerPage(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Auto Modes"; }

  void present(bool* running) override;

  enum class Event {
    NONE = 0,
    NEW_AUTO_MODE,
    RENAME_AUTO_MODE,
    DUPLICATE_AUTO_MODE,
  };

  Event lastPresentEvent() const noexcept { return m_event; }

  const std::string& eventAutoMode() const noexcept { return m_eventAutoMode; }

 private:
  Event m_event = Event::NONE;
  std::string m_eventAutoMode;
};
