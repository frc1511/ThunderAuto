#include <ThunderAuto/pages/actions_page.hpp>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.hpp>

void ActionsPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(name(), running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }

  ImGuiStyle& style = ImGui::GetStyle();

  ProjectState state = m_history.current_state();

  std::vector<std::string>& actions = state.actions();

  for (std::size_t i = 0; i < actions.size(); ++i) {
    std::string& name = actions.at(i);

    char buf[256];
    strncpy(buf, name.c_str(), name.length());
    buf[name.length()] = '\0';

    // D:
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.f - style.ItemSpacing.x);

    ImGui::InputText(("##action_name_" + std::to_string(i)).c_str(), buf, 256,

                     // This is a hack to stop tab navigation between items,
                     // because that breaks history.
                     ImGuiInputTextFlags_AllowTabInput);

    if (ImGui::IsItemActivated()) {
      m_history.start_long_edit();
    }

    if (ImGui::IsItemActive()) {
      name = buf;
      m_history.add_state(state);
    }

    if (ImGui::IsItemDeactivated()) {
      if (ImGui::IsItemDeactivatedAfterEdit())
        m_history.finish_long_edit();
      else
        m_history.discard_long_edit();
    }

    // TODO: This sucks.
    ImVec2 region_max = ImGui::GetContentRegionAvail();

    ImGui::SameLine(region_max.x - 75.0f);
    ImGui::Text("1 << %d", (int)i);

    ImGui::SameLine(region_max.x - 25.0f);
    ImGui::PushID((void*)(intptr_t)i);

    if (ImGui::Button(ICON_FA_TRASH)) {
      for (auto& path : state.paths()) {
        for (CurvePoint& point : path.second.points()) {
          point.remove_action(i, true);
        }
      }

      actions.erase(actions.begin() + i);
      m_history.add_state(state);
    }

    ImGui::PopID();
  }

  {
    ImGuiScopedDisabled disabled(actions.size() >= 32);

    if (ImGui::Selectable("+ New Action", false)) {
      actions.push_back("Action");
      m_history.add_state(state);
    }
  }

  ImGui::End();
}
