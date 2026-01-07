#include <ThunderAuto/Popups/RenameActionPopup.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/Error.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void RenameActionPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(RENAME_ACTION_POPUP_START_WIDTH), GET_UISIZE(RENAME_ACTION_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();

  {
    auto scopedField = ImGui::ScopedField::Builder("ActionName Name").build();

    ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
      // [a-zA-Z0-9_ ]
      if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
      }
      return 0;
    };

    ImGui::InputText("##Trajectory Name", m_newActionNameBuffer, sizeof(m_newActionNameBuffer),
                     ImGuiInputTextFlags_CallbackCharFilter, callback);
  }

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const std::string newActionName = m_newActionNameBuffer;

  const bool wasActionNameChanged = newActionName != m_oldActionName;

  const bool isNewActionNameAlreadyTaken = wasActionNameChanged && state.actions.contains(newActionName);
  const bool isNewActionNameEmpty = newActionName.empty();

  const bool isRenameDisabled = isNewActionNameAlreadyTaken || isNewActionNameEmpty;

  auto renameDisabled = ImGui::Scoped::Disabled(isRenameDisabled);

  if (ImGui::Button("Rename")) {
    if (wasActionNameChanged) {
      ThunderAutoLogger::Info("Renaming action '{}' to '{}'", m_oldActionName, newActionName);

      state.renameAction(m_oldActionName, newActionName);
      m_history.addState(state);
    }

    reset();
    *running = false;
  }

  // Tooltips

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
    if (isNewActionNameAlreadyTaken) {
      ImGui::SetTooltip("A trajectory with this name already exists.");
    } else if (isNewActionNameEmpty) {
      ImGui::SetTooltip("Trajectory name can not be empty.");
    }
  }
}
