#include <ThunderAuto/Popups/NewActionPopup.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/Error.hpp>
#include <IconsLucide.h>
#include <imgui.h>
#include <imgui_raii.h>

void NewActionPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(NEW_ACTION_POPUP_START_WIDTH), GET_UISIZE(NEW_ACTION_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();

  presentNameProperty();

  ImGui::Separator();

  presentTypeProperty();

  if (m_actionInfo.type() == ThunderAutoActionType::SEQUENTIAL_ACTION_GROUP) {
    presentSequentialActionGroup(state);
  } else if (m_actionInfo.type() == ThunderAutoActionType::CONCURRENT_ACTION_GROUP) {
    presentConcurrentActionGroup(state);
  }

  ImGui::Separator();

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const std::string actionName = m_actionNameBuffer;

  const bool isActionNameAlreadyTaken = state.actions.contains(actionName);
  const bool isActionNameEmpty = actionName.empty();

  const bool isCreateDisabled = isActionNameAlreadyTaken || isActionNameEmpty;

  auto createDisabled = ImGui::Scoped::Disabled(isCreateDisabled);

  if (ImGui::Button("Create")) {
    ThunderAutoLogger::Info("Creating new action '{}'", actionName);

    // Add trajectory

    state.addAction(actionName, m_actionInfo);

    // Apply changes

    m_history.addState(state);
    m_newActionName = actionName;

    reset();
    *running = false;
  }

  // Tooltips

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
    if (isActionNameAlreadyTaken) {
      ImGui::SetTooltip("An action with this name already exists.");
    } else if (isActionNameEmpty) {
      ImGui::SetTooltip("Action name can not be empty.");
    }
  }
}

void NewActionPopup::presentNameProperty() {
  auto scopedField = ImGui::ScopedField::Builder("Action Name").build();

  ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
    // [a-zA-Z0-9_ ]
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
      return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
    }
    return 0;
  };

  ImGui::InputText("##Action Name", m_actionNameBuffer, sizeof(m_actionNameBuffer),
                   ImGuiInputTextFlags_CallbackCharFilter, callback);
}

void NewActionPopup::presentTypeProperty() {
  auto scopedField = ImGui::ScopedField::Builder("Type").build();

  using enum ThunderAutoActionType;

  const char* actionTypeStr = ThunderAutoActionTypeToString(m_actionInfo.type());
  if (auto scopedCombo = ImGui::Scoped::Combo("##Action Type", actionTypeStr)) {
    ThunderAutoActionType actionType = m_actionInfo.type();
    if (ImGui::Selectable(ThunderAutoActionTypeToString(COMMAND), actionType == COMMAND)) {
      actionType = COMMAND;
    }
    if (ImGui::Selectable(ThunderAutoActionTypeToString(SEQUENTIAL_ACTION_GROUP),
                          actionType == SEQUENTIAL_ACTION_GROUP)) {
      actionType = SEQUENTIAL_ACTION_GROUP;
    }
    if (ImGui::Selectable(ThunderAutoActionTypeToString(CONCURRENT_ACTION_GROUP),
                          actionType == CONCURRENT_ACTION_GROUP)) {
      actionType = CONCURRENT_ACTION_GROUP;
    }
    if (m_actionInfo.type() != actionType) {
      m_actionInfo.setType(actionType);
    }
  }
}

void NewActionPopup::presentSequentialActionGroup(const ThunderAutoProjectState& state) {
  auto scopedField = ImGui::ScopedField::Builder("Group Actions").build();

  {
    auto scopedChildWindow = ImGui::Scoped::ChildWindow(
        "##Action Group", ImVec2(0.f, GET_UISIZE(NEW_ACTION_POPUP_CHILD_WINDOW_START_SIZE_Y)),
        ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders, ImGuiWindowFlags_NoSavedSettings);

    size_t i = 0;
    const std::vector<std::string>& actionGroup = m_actionInfo.actionGroup();
    for (auto actionIt = actionGroup.begin(); actionIt != actionGroup.end(); actionIt++, i++) {
      auto scopedID = ImGui::Scoped::ID(i);

      (void)ImGui::Selectable(actionIt->c_str(), false, ImGuiSelectableFlags_AllowOverlap);

      if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
        bool moveUp = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y > 0.f;
        if (moveUp && i < actionGroup.size() - 1) {
          (void)m_actionInfo.swapGroupActionWithNext(*actionIt);
        } else if (!moveUp && i > 0) {
          (void)m_actionInfo.swapGroupActionWithPrevious(*actionIt);
        } else {
          ImGui::ResetMouseDragDelta();
        }
      }

      ImGui::SameLine();

      const ImGuiStyle& style = ImGui::GetStyle();
      const float removeButtonWidthNeeded = ImGui::CalcTextSize(ICON_LC_TRASH).x + style.ItemSpacing.x;
      const float removeButtonCursorOffset = ImGui::GetContentRegionAvail().x - removeButtonWidthNeeded;
      if (removeButtonCursorOffset > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + removeButtonCursorOffset);
      }

      auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
      auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
      auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

      if (ImGui::SmallButton(ICON_LC_TRASH)) {
        (void)m_actionInfo.removeGroupAction(*actionIt);
        break;
      }
    }
  }

  const float buttonWidth = ImGui::GetContentRegionAvail().x;
  if (ImGui::Button("+ Add Action", ImVec2(buttonWidth, 0.f))) {
    ImGui::OpenPopup("Add Action To Group");
  }

  if (auto scopedAddActionPopup = ImGui::Scoped::Popup("Add Action To Group")) {
    std::string selectedAction;

    if (auto scopedCombo = ImGui::Scoped::Combo("##Group Add Action Combo", nullptr)) {
      for (const std::string& addActionName : state.actionsOrder) {
        auto scopedDisabled = ImGui::Scoped::Disabled(m_actionInfo.hasGroupAction(addActionName));
        if (ImGui::Selectable(addActionName.c_str())) {
          selectedAction = addActionName;
        }
      }
    }

    if (!selectedAction.empty()) {
      (void)m_actionInfo.addGroupAction(selectedAction);
      ImGui::CloseCurrentPopup();
    }
  }
}

void NewActionPopup::presentConcurrentActionGroup(const ThunderAutoProjectState& state) {
  auto scopedField = ImGui::ScopedField::Builder("Group Actions").build();

  auto scopedChildWindow = ImGui::Scoped::ChildWindow(
      "##Action Group", ImVec2(0.f, GET_UISIZE(NEW_ACTION_POPUP_CHILD_WINDOW_START_SIZE_Y)),
      ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders, ImGuiWindowFlags_NoSavedSettings);

  for (const std::string& action : state.actionsOrder) {
    bool isPresent = m_actionInfo.hasGroupAction(action);
    if (ImGui::Checkbox(action.c_str(), &isPresent)) {
      if (isPresent) {
        m_actionInfo.addGroupAction(action);
      } else {
        m_actionInfo.removeGroupAction(action);
      }
    }
  }
}

