#include <ThunderAuto/Popups/AddAutoModeStepPopup.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/Error.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void AddAutoModeStepPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(ADD_AUTO_MODE_STEP_POPUP_START_WIDTH),
                                  GET_UISIZE(ADD_AUTO_MODE_STEP_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();

  presentStepTypeProperty();

  if (m_stepType != ThunderAutoModeStepType::UNKNOWN) {
    ImGui::Separator();
    presentStepProperties(state);
  }

  ImGui::Separator();

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const char* addDisabledReason = getAddButtonDisabledReason();
  bool isAddDisabled = addDisabledReason != nullptr;

  auto scopedDisabled = ImGui::Scoped::Disabled(isAddDisabled);
  if (ImGui::Button("Add")) {
    addStep(state);
    reset();
    *running = false;
  }

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && isAddDisabled) {
    ImGui::SetTooltip("%s", addDisabledReason);
  }
}

void AddAutoModeStepPopup::presentStepTypeProperty() {
  const char* currentStepTypeName = "";
  if (m_stepType != ThunderAutoModeStepType::UNKNOWN) {
    currentStepTypeName = ThunderAutoModeStepTypeToString(m_stepType);
  }

  {
    auto scopedField = ImGui::ScopedField::Builder("Step Type").build();

    if (auto scopedCombo = ImGui::Scoped::Combo("##Step Type", currentStepTypeName)) {
      using enum ThunderAutoModeStepType;
      for (int i = (int)ACTION; i <= (int)BRANCH_SWITCH; ++i) {
        ThunderAutoModeStepType stepType = static_cast<ThunderAutoModeStepType>(i);
        const char* stepTypeName = ThunderAutoModeStepTypeToString(stepType);

        bool isSelected = (m_stepType == stepType);
        if (ImGui::Selectable(stepTypeName, isSelected) && !isSelected) {
          reset();
          m_stepType = stepType;
        }
      }
    }
  }
}

void AddAutoModeStepPopup::presentStepProperties(const ThunderAutoProjectState& state) {
  switch (m_stepType) {
    using enum ThunderAutoModeStepType;
    case ACTION:
      presentActionStepProperties(state);
      break;
    case TRAJECTORY:
      presentTrajectoryStepProperties(state);
      break;
    case BRANCH_BOOL:
    case BRANCH_SWITCH:
      presentBranchStepProperties(state);
      break;
    default:
      break;
  }
}

void AddAutoModeStepPopup::presentActionStepProperties(const ThunderAutoProjectState& state) {
  auto scopedField = ImGui::ScopedField::Builder("Action").build();

  if (auto scopedCombo = ImGui::Scoped::Combo("##Action Name", m_actionName.c_str())) {
    for (const std::string& action : state.actionsOrder) {
      bool isActionSelected = action == m_actionName;
      if (ImGui::Selectable(action.c_str(), isActionSelected)) {
        m_actionName = action;
      }
    }
  }
}

void AddAutoModeStepPopup::presentTrajectoryStepProperties(const ThunderAutoProjectState& state) {
  auto scopedField = ImGui::ScopedField::Builder("Trajectory").build();

  if (auto scopedCombo = ImGui::Scoped::Combo("##Trajectory Name", m_trajectoryName.c_str())) {
    for (const auto& [trajectoryName, trajectory] : state.trajectories) {
      bool isTrajectorySelected = trajectoryName == m_trajectoryName;
      if (ImGui::Selectable(trajectoryName.c_str(), isTrajectorySelected)) {
        m_trajectoryName = trajectoryName;
      }
    }
  }
}

void AddAutoModeStepPopup::presentBranchStepProperties(const ThunderAutoProjectState& state) {
  {
    auto scopedField = ImGui::ScopedField::Builder("Condition Name").build();

    ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
      // [a-zA-Z0-9_ ]
      if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
      }
      return 0;
    };

    ImGui::InputText("##Condition Name", m_conditionNameBuffer, sizeof(m_conditionNameBuffer),
                     ImGuiInputTextFlags_CallbackCharFilter, callback);
  }
}

const char* AddAutoModeStepPopup::getAddButtonDisabledReason() {
  const char* addDisabledReason = nullptr;

  switch (m_stepType) {
    using enum ThunderAutoModeStepType;
    case ACTION:
      if (m_actionName.empty()) {
        addDisabledReason = "No action selected";
      }
      break;
    case TRAJECTORY:
      if (m_trajectoryName.empty()) {
        addDisabledReason = "No trajectory selected";
      }
      break;
    case BRANCH_BOOL:
    case BRANCH_SWITCH:
      if (m_conditionNameBuffer[0] == '\0') {
        addDisabledReason = "No condition name specified";
      }
      break;
    default:
      addDisabledReason = "No step type selected";
      break;
  }

  return addDisabledReason;
}

void AddAutoModeStepPopup::addStep(ThunderAutoProjectState& state) {
  std::unique_ptr<ThunderAutoModeStep> step;

  switch (m_stepType) {
    using enum ThunderAutoModeStepType;
    case ACTION:
      step = makeActionStep();
      break;
    case TRAJECTORY:
      step = makeTrajectoryStep();
      break;
    case BRANCH_BOOL:
      step = makeBoolBranchStep();
      break;
    case BRANCH_SWITCH:
      step = makeSwitchBranchStep();
      break;
    default:
      break;
  }

  ThunderAutoAssert(step != nullptr, "Failed to create auto mode step");

  ThunderAutoMode& autoMode = state.currentAutoMode();
  autoMode.steps.push_back(std::move(step));

  m_history.addState(state);
}

std::unique_ptr<ThunderAutoModeStep> AddAutoModeStepPopup::makeActionStep() {
  auto step = std::make_unique<ThunderAutoModeActionStep>();
  step->actionName = m_actionName;
  return step;
}

std::unique_ptr<ThunderAutoModeStep> AddAutoModeStepPopup::makeTrajectoryStep() {
  auto step = std::make_unique<ThunderAutoModeTrajectoryStep>();
  step->trajectoryName = m_trajectoryName;
  return step;
}

std::unique_ptr<ThunderAutoModeStep> AddAutoModeStepPopup::makeBoolBranchStep() {
  auto step = std::make_unique<ThunderAutoModeBoolBranchStep>();
  step->conditionName = m_conditionNameBuffer;
  return step;
}

std::unique_ptr<ThunderAutoModeStep> AddAutoModeStepPopup::makeSwitchBranchStep() {
  auto step = std::make_unique<ThunderAutoModeSwitchBranchStep>();
  step->conditionName = m_conditionNameBuffer;
  return step;
}
