#include <ThunderAuto/Popups/LinkTrajectoryEndBehaviorPopup.hpp>

#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <imgui.h>
#include <imgui_raii.h>

// void LinkTrajectoryEndBehaviorPopup::prepareForOpen(bool startBehaviorLink /*= true*/) {
//   reset();
//   const ThunderAutoProjectState& state = m_history.currentState();
//   const ThunderAutoTrajectorySkeleton& trajectory = state.currentTrajectory();
//   m_initialLinkName = m_selectedLinkName =
//       startBehaviorLink ? trajectory.startBehaviorLinkName() : trajectory.endBehaviorLinkName();
//   m_willLinkStartBehavior = startBehaviorLink;
// }

void LinkTrajectoryEndBehaviorPopup::setTrajectoryName(const std::string& trajectoryName, bool startBehavior) noexcept {
  m_trajectoryName = trajectoryName;

  const ThunderAutoProjectState& state = m_history.currentState();
  const ThunderAutoTrajectorySkeleton& trajectory = state.trajectories.at(trajectoryName);
  m_initialStartLinkName = trajectory.startBehaviorLinkName();
  m_initialEndLinkName = trajectory.endBehaviorLinkName();

  m_startBehavior = startBehavior;
  m_selectedLinkName = m_startBehavior ? m_initialStartLinkName : m_initialEndLinkName;
}

void LinkTrajectoryEndBehaviorPopup::setupForCurrentTrajectory(bool startBehavior) noexcept {
  const ThunderAutoProjectState& state = m_history.currentState();
  setTrajectoryName(state.currentTrajectoryName(), startBehavior);
}

void LinkTrajectoryEndBehaviorPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(LINK_TRAJECTORY_END_BEHAVIOR_POPUP_START_WIDTH),
                                  GET_UISIZE(LINK_TRAJECTORY_END_BEHAVIOR_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();

  {
    auto scopedField = ImGui::ScopedField::Builder("Behavior To Link").build();

    const char* options[] = {"Start", "End"};
    int currentOption = m_startBehavior ? 0 : 1;

    if (ImGui::Combo("##Behavior", &currentOption, options, IM_ARRAYSIZE(options))) {
      bool startBehavior = (currentOption == 0);
      if (m_startBehavior != startBehavior) {
        m_startBehavior = startBehavior;
        m_selectedLinkName = m_startBehavior ? m_initialStartLinkName : m_initialEndLinkName;
        m_createNewLink = false;
        m_newLinkNameBuffer[0] = '\0';
      }
    }
  }

  {
    auto scopedField = ImGui::ScopedField::Builder("Link").build();

    const char* comboTitle = m_selectedLinkName.c_str();

    if (m_createNewLink) {
      comboTitle = "+ New Link";
    } else if (m_selectedLinkName.empty()) {
      comboTitle = "<none>";
    }

    if (auto scopedCombo = ImGui::Scoped::Combo("##link", comboTitle)) {
      for (const std::string& linkName : state.trajectoryEndBehaviorLinks) {
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

      if (ImGui::Selectable("<none>", m_selectedLinkName.empty())) {
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
    } else if (state.trajectoryEndBehaviorLinks.contains(newLinkName)) {
      isConfirmDisabled = true;
      confirmDisabledReason = "A link with this name already exists.";
    } else if (newLinkName == "None") {
      isConfirmDisabled = true;
      confirmDisabledReason = "\"None\" is a reserved link name.";
    }
  }

  auto confirmDisabled = ImGui::Scoped::Disabled(isConfirmDisabled);
  if (ImGui::Button("Confirm")) {
    state.trajectorySelect(m_trajectoryName);
    ThunderAutoTrajectorySkeleton& skeleton = state.trajectories.at(m_trajectoryName);

    if (m_createNewLink) {
      const std::string newLinkName = m_newLinkNameBuffer;
      ThunderAutoLogger::Info("Create new trajectory end behavior link '{}'", newLinkName);
      state.trajectoryEndBehaviorLinks.insert(newLinkName);
      if (m_startBehavior) {
        skeleton.setStartBehaviorLinkName(newLinkName);
      } else {
        skeleton.setEndBehaviorLinkName(newLinkName);
      }

      m_history.addState(state);

    } else if (m_selectedLinkName != (m_startBehavior ? m_initialStartLinkName : m_initialEndLinkName)) {
      ThunderAutoLogger::Info("Set trajectory end behavior link to '{}'", m_selectedLinkName);
      if (m_startBehavior) {
        skeleton.setStartBehaviorLinkName(m_selectedLinkName);
      } else {
        skeleton.setEndBehaviorLinkName(m_selectedLinkName);
      }
      state.currentTrajectoryUpdateEndBehaviorFromLinks();

      m_history.addState(state);
    }

    reset();
    *running = false;
  }

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled) &&
      isConfirmDisabled) {
    ImGui::SetTooltip("%s", confirmDisabledReason);
  }
}
