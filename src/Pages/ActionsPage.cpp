#include <ThunderAuto/Pages/ActionsPage.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/FontLibrary.hpp>
#include <ThunderAuto/Error.hpp>
#include <IconsLucide.h>
#include <imgui_raii.h>

void ActionsPage::present(bool* running) {
  m_event = Event::NONE;

  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(ACTIONS_PAGE_START_WIDTH), GET_UISIZE(ACTIONS_PAGE_START_HEIGHT)),
      ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || !*running)
    return;

  ThunderAutoProjectState state = m_history.currentState();

  std::vector<std::string> actionsOrder = state.actionsOrder;
  for (size_t actionIndex = 0; actionIndex < actionsOrder.size(); actionIndex++) {
    const std::string& actionName = actionsOrder[actionIndex];
    ThunderAutoAction& actionInfo = state.getAction(actionName);

    {
      auto scopedID = ImGui::Scoped::ID(actionIndex);
      auto scopedPadding = ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);

      const float spacingX = std::max(ImGui::GetContentRegionAvail().x, 1.f);
      const float spacingY = GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y) / 3.f;
      (void)ImGui::InvisibleButton("Drag Separator", ImVec2(spacingX, spacingY));

      if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Action")) {
          std::string payloadActionName = reinterpret_cast<const char*>(payload->Data);
          if (actionName != payloadActionName) {
            ThunderAutoLogger::Info("Move action '{}' before action '{}'", payloadActionName, actionName);
            state.moveActionBeforeOther(payloadActionName, actionName);
            m_history.addState(state);
          }
        }
      }
    }

    {
      ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);
      const ImGuiTreeNodeFlags treeNodeFlags =
          ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth;
      auto scopedTreeNode = ImGui::Scoped::TreeNodeEx(actionName.c_str(), treeNodeFlags);
      ImGui::PopStyleVar();

      if (auto popup = ImGui::Scoped::PopupContextItem()) {
        if (ImGui::MenuItem(ICON_LC_PENCIL "  Rename")) {
          m_event = Event::RENAME_ACTION;
          m_eventActionName = actionName;
        }
        if (ImGui::MenuItem(ICON_LC_TRASH "  Delete")) {
          state.removeAction(actionName);
          m_history.addState(state);
        }
      }

      if (auto scopedDragSource = ImGui::Scoped::DragDropSource()) {
        ImGui::SetDragDropPayload("Action", actionName.c_str(), actionName.size() + 1);
        ImGui::Text("%s", actionName.c_str());
      }

      if (scopedTreeNode) {
        using enum ThunderAutoActionType;

        const float fieldLeftColumnWidth = GET_UISIZE(FIELD_NORMAL_LEFT_COLUMN_WIDTH) * 0.5f;

        {
          auto scopedField =
              ImGui::ScopedField::Builder("Type").leftColumnWidth(fieldLeftColumnWidth).build();

          const char* actionTypeStr = ThunderAutoActionTypeToString(actionInfo.type());
          if (auto scopedCombo = ImGui::Scoped::Combo("##Action Type", actionTypeStr)) {
            ThunderAutoActionType actionType = actionInfo.type();
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
            if (actionInfo.type() != actionType) {
              actionInfo.setType(actionType);
              m_history.addState(state);
            }
          }
        }

        if (actionInfo.type() == SEQUENTIAL_ACTION_GROUP || actionInfo.type() == CONCURRENT_ACTION_GROUP) {
          auto scopedField =
              ImGui::ScopedField::Builder("Actions").leftColumnWidth(fieldLeftColumnWidth).build();

          {
            auto scopedChildWindow = ImGui::Scoped::ChildWindow(
                "Action Group", ImVec2(0.f, GET_UISIZE(ACTIONS_PAGE_ACTIONS_GROUP_CHILD_WINDOW_START_SIZE_Y)),
                ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders, ImGuiWindowFlags_NoSavedSettings);

            size_t i = 0;
            const std::vector<std::string>& actionGroup = actionInfo.actionGroup();
            for (auto actionIt = actionGroup.begin(); actionIt != actionGroup.end(); actionIt++, i++) {
              auto scopedID = ImGui::Scoped::ID(i);

              (void)ImGui::Selectable(actionIt->c_str(), false, ImGuiSelectableFlags_AllowOverlap);

              if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
                bool moveUp = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y > 0.f;
                bool wasMoved = false;
                if (moveUp && i < actionGroup.size() - 1) {
                  wasMoved = actionInfo.swapGroupActionWithNext(*actionIt);
                } else if (!moveUp && i > 0) {
                  wasMoved = actionInfo.swapGroupActionWithPrevious(*actionIt);
                }

                if (wasMoved) {
                  m_history.addState(state);
                  ImGui::ResetMouseDragDelta();
                }
              }

              ImGui::SameLine();

              const ImGuiStyle& style = ImGui::GetStyle();
              const float removeButtonWidthNeeded =
                  ImGui::CalcTextSize(ICON_LC_TRASH).x + style.ItemSpacing.x;
              const float removeButtonCursorOffset =
                  ImGui::GetContentRegionAvail().x - removeButtonWidthNeeded;
              if (removeButtonCursorOffset > 0) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + removeButtonCursorOffset);
              }

              auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
              auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
              auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

              if (ImGui::SmallButton(ICON_LC_TRASH)) {
                const bool wasRemoved = actionInfo.removeGroupAction(*actionIt);
                if (wasRemoved) {
                  m_history.addState(state);
                }
                break;
              }
            }
          }
          if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Action")) {
              std::string payloadActionName = reinterpret_cast<const char*>(payload->Data);
              const bool wasAdded = actionInfo.addGroupAction(payloadActionName) &&
                                    verifyAddedGroupAction(state, actionName, payloadActionName);
              if (wasAdded) {
                m_history.addState(state);
              }
            }
          }

          const float buttonWidth = ImGui::GetContentRegionAvail().x;
          if (ImGui::Button("+ Add Action", ImVec2(buttonWidth, 0.f))) {
            ImGui::OpenPopup("Add Action To Group");
          }

          if (auto scopedPopup = ImGui::Scoped::Popup("Add Action To Group")) {
            std::string selectedAction;
            bool newAction = false;

            if (auto scopedCombo = ImGui::Scoped::Combo("##Group Add Action Combo", nullptr)) {
              for (const std::string& addActionName : actionsOrder) {
                auto scopedDisabled = ImGui::Scoped::Disabled(actionName == addActionName ||
                                                              actionInfo.hasGroupAction(addActionName));
                if (ImGui::Selectable(addActionName.c_str())) {
                  selectedAction = addActionName;
                }
              }

              ImGui::Separator();
              if (ImGui::Selectable("+ New Action")) {
                newAction = true;
              }
            }

            if (!selectedAction.empty()) {
              const bool wasAdded = actionInfo.addGroupAction(selectedAction) &&
                                    verifyAddedGroupAction(state, actionName, selectedAction);

              if (wasAdded) {
                m_history.addState(state);
              }
              ImGui::CloseCurrentPopup();

            } else if (newAction) {
              ImGui::CloseCurrentPopup();
              m_event = Event::NEW_ACTION_ADD_TO_GROUP;
              m_eventActionName = actionName;
            }
          }
        }
      }
    }

    if (actionIndex == actionsOrder.size() - 1) {
      auto scopedID = ImGui::Scoped::ID("Bottom Drag Target");

      const float spacingX = std::max(ImGui::GetContentRegionAvail().x, 1.f);
      const float spacingY = GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y) / 3.f;
      (void)ImGui::InvisibleButton("Drag Separator", ImVec2(spacingX, spacingY));

      if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Action")) {
          std::string payloadActionName = reinterpret_cast<const char*>(payload->Data);
          if (actionName != payloadActionName) {
            ThunderAutoLogger::Info("Move action '{}' after action '{}'", payloadActionName, actionName);
            state.moveActionAfterOther(payloadActionName, actionName);
            m_history.addState(state);
          }
        }
      }
    }
  }

  ImGui::Separator();

  if (ImGui::Button("+ New Action", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
    m_event = Event::NEW_ACTION;
    m_eventActionName.clear();
  }
}

bool ActionsPage::verifyAddedGroupAction(const ThunderAutoProjectState& state,
                                         const std::string& groupActionName,
                                         const std::string& addedActionName) {
  m_eventActionRecursionPath = state.findActionRecursionPath(addedActionName);
  if (!m_eventActionRecursionPath.empty()) {
    m_event = Event::INVALID_OPERATION_RECURSIVE_ACTION;
    m_eventActionName = groupActionName;
    return false;
  }

  return true;
}
