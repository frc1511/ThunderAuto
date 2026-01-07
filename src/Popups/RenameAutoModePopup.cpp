#include <ThunderAuto/Popups/RenameAutoModePopup.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void RenameAutoModePopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(RENAME_AUTO_MODE_POPUP_START_WIDTH),
                                  GET_UISIZE(RENAME_AUTO_MODE_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();
  std::map<std::string, ThunderAutoMode>& autoModes = state.autoModes;
  {
    auto scopedField = ImGui::ScopedField::Builder("Auto Mode Name").build();

    ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
      // [a-zA-Z0-9_ ]
      if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
      }
      return 0;
    };

    ImGui::InputText("##Auto Mode Name", m_newAutoModeNameBuffer, sizeof(m_newAutoModeNameBuffer),
                     ImGuiInputTextFlags_CallbackCharFilter, callback);
  }

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const std::string newAutoModeName = m_newAutoModeNameBuffer;

  const bool wasAutoModeNameChanged = newAutoModeName != m_oldAutoModeName;

  const bool isNewAutoModeNameAlreadyTaken =
      wasAutoModeNameChanged && autoModes.contains(newAutoModeName);
  const bool isNewAutoModeNameEmpty = newAutoModeName.empty();

  const bool isRenameDisabled = isNewAutoModeNameAlreadyTaken || isNewAutoModeNameEmpty;

  auto renameDisabled = ImGui::Scoped::Disabled(isRenameDisabled);

  if (ImGui::Button("Rename")) {
    if (wasAutoModeNameChanged) {
      ThunderAutoLogger::Info("Renaming auto mode '{}' to '{}'", m_oldAutoModeName, newAutoModeName);

      state.autoModeRename(m_oldAutoModeName, newAutoModeName);
      m_history.addState(state);
    }

    reset();
    *running = false;
  }

  // Tooltips

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
    if (isNewAutoModeNameAlreadyTaken) {
      ImGui::SetTooltip("An auto mode with this name already exists.");
    } else if (isNewAutoModeNameEmpty) {
      ImGui::SetTooltip("Auto mode name can not be empty.");
    }
  }
}
