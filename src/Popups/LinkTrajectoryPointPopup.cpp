#include <ThunderAuto/Popups/LinkTrajectoryPointPopup.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void LinkTrajectoryPointPopup::prepareForOpen() {
  reset();
  ThunderAutoProjectState state = m_history.currentState();
  m_initialLinkName = m_selectedLinkName = state.currentTrajectorySelectedWaypoint().linkName();
}

void LinkTrajectoryPointPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(LINK_TRAJECTORY_POINT_POPUP_START_WIDTH),
                                  GET_UISIZE(LINK_TRAJECTORY_POINT_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();

  {
    auto scopedField = ImGui::ScopedField::Builder("Link").build();

    const char* comboTitle = m_selectedLinkName.c_str();

    if (m_createNewLink) {
      comboTitle = "+ New Link";
    } else if (m_selectedLinkName.empty()) {
      comboTitle = "None";
    }

    if (auto scopedCombo = ImGui::Scoped::Combo("##link", comboTitle)) {
      for (const std::string& linkName : state.waypointLinks) {
        bool isSelected = (m_selectedLinkName == linkName);
        if (ImGui::Selectable(linkName.c_str(), isSelected)) {
          m_selectedLinkName = linkName;
          m_createNewLink = false;
        }
      }

      ImGui::Separator();

      if (ImGui::Selectable("+ New Link", m_createNewLink)) {
        m_selectedLinkName.clear();
        m_createNewLink = true;
      }

      ImGui::Separator();

      if (ImGui::Selectable("None", m_selectedLinkName.c_str())) {
        m_selectedLinkName.clear();
        m_createNewLink = false;
      }
    }
  }

  if (m_createNewLink) {
    auto scopedField = ImGui::ScopedField::Builder("Link Name").build();

    ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
      // [a-zA-Z0-9_ ]
      if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
      }
      return 0;
    };

    ImGui::InputText("##input", m_newLinkNameBuffer, sizeof(m_newLinkNameBuffer),
                     ImGuiInputTextFlags_CallbackCharFilter, callback);
  }

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  bool isConfirmDisabled = false;
  const char* confirmDisabledReason = "";
  if (m_createNewLink) {
    const std::string newLinkName = m_newLinkNameBuffer;
    if (newLinkName.empty()) {
      isConfirmDisabled = true;
      confirmDisabledReason = "Link name can not be empty.";
    } else if (state.waypointLinks.contains(newLinkName)) {
      isConfirmDisabled = true;
      confirmDisabledReason = "A link with this name already exists.";
    } else if (newLinkName == "None") {
      isConfirmDisabled = true;
      confirmDisabledReason = "\"None\" is a reserved link name.";
    }
  }

  auto confirmDisabled = ImGui::Scoped::Disabled(isConfirmDisabled);
  if (ImGui::Button("Confirm")) {
    ThunderAutoTrajectorySkeletonWaypoint& point = state.currentTrajectorySelectedWaypoint();

    if (m_createNewLink) {
      const std::string newLinkName = m_newLinkNameBuffer;
      ThunderAutoLogger::Info("Create new waypoint link '{}'", newLinkName);
      state.waypointLinks.insert(newLinkName);
      point.setLinkName(newLinkName);

      m_history.addState(state);

    } else if (m_selectedLinkName != m_initialLinkName) {
      ThunderAutoLogger::Info("Set waypoint link to '{}'", m_selectedLinkName);
      point.setLinkName(m_selectedLinkName);
      state.trajectoryUpdateSelectedWaypointFromLink();

      m_history.addState(state);
    }

    reset();
    *running = false;
  }

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled) && isConfirmDisabled) {
    ImGui::SetTooltip("%s", confirmDisabledReason);
  }
}
