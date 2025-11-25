#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <ThunderLibCore/Auto/ThunderAutoMode.hpp>
#include <string>

using namespace thunder::core;

class AddAutoModeStepPopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  ThunderAutoModeStepType m_stepType = ThunderAutoModeStepType::UNKNOWN;

  std::string m_actionName;
  std::string m_trajectoryName;
  char m_conditionNameBuffer[256] = "";

 public:
  AddAutoModeStepPopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Add Step to Auto Mode"; }

  void present(bool* running) override;

  void reset() noexcept {
    m_stepType = ThunderAutoModeStepType::UNKNOWN;
    m_actionName.clear();
    m_trajectoryName.clear();
    m_conditionNameBuffer[0] = '\0';
  }

 private:
  void presentStepTypeProperty();

  void presentStepProperties(const ThunderAutoProjectState& state);
  void presentActionStepProperties(const ThunderAutoProjectState& state);
  void presentTrajectoryStepProperties(const ThunderAutoProjectState& state);
  void presentBranchStepProperties(const ThunderAutoProjectState& state);

  const char* getAddButtonDisabledReason();

  void addStep(ThunderAutoProjectState& state);
  std::unique_ptr<ThunderAutoModeStep> makeActionStep();
  std::unique_ptr<ThunderAutoModeStep> makeTrajectoryStep();
  std::unique_ptr<ThunderAutoModeStep> makeBoolBranchStep();
  std::unique_ptr<ThunderAutoModeStep> makeSwitchBranchStep();
};
