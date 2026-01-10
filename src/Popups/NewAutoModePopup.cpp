#include <ThunderAuto/Popups/NewAutoModePopup.hpp>

#include <ThunderLibCore/Auto/ThunderAutoMode.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <imgui.h>
#include <imgui_raii.h>

using namespace thunder::core;

void NewAutoModePopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(NEW_AUTO_MODE_POPUP_START_WIDTH), GET_UISIZE(NEW_AUTO_MODE_POPUP_START_HEIGHT)));

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

    ImGui::InputText("##Auto Mode Name", m_autoModeNameBuffer, sizeof(m_autoModeNameBuffer),
                     ImGuiInputTextFlags_CallbackCharFilter, callback);
  }

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const std::string autoModeName = m_autoModeNameBuffer;

  const bool isAutoModeNameAlreadyTaken = autoModes.contains(autoModeName);
  const bool isAutoModeNameEmpty = autoModeName.empty();

  const bool isCreateDisabled = isAutoModeNameAlreadyTaken || isAutoModeNameEmpty;

  auto createDisabled = ImGui::Scoped::Disabled(isCreateDisabled);

  if (ImGui::Button("Create")) {
    ThunderAutoLogger::Info("Creating new auto mode '{}'", autoModeName);

    // Add auto mode

    state.autoModes.emplace(autoModeName, ThunderAutoMode{});
    state.autoModeSelect(autoModeName);

    // Apply changes

    m_history.addState(state);

    reset();
    *running = false;
  }

  // Tooltips

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
    if (isAutoModeNameAlreadyTaken) {
      ImGui::SetTooltip("An auto mode with this name already exists.");
    } else if (isAutoModeNameEmpty) {
      ImGui::SetTooltip("Auto mode name can not be empty.");
    }
  }
}
