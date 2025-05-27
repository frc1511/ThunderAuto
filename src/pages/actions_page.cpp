#include <ThunderAuto/pages/actions_page.hpp>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.hpp>
#include <ThunderAuto/font_library.hpp>

void ActionsPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(ACTIONS_PAGE_START_WIDTH),
                                  GET_UISIZE(ACTIONS_PAGE_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);
  ImGui::Scoped s = ImGui::Scoped::Window(name(), running);
  if (!s)
    return;

  ProjectState state = m_history.current_state();

  size_t i = 0, num_actions;
  while ((num_actions = state.actions().size()) > i) {
    present_action_name_input(state, i);

    ImGui::SameLine();
    present_action_id(i, num_actions);

    ImGui::SameLine();
    present_action_delete_button(state, i);

    ++i;
  }

  present_new_action_button(state);
}

void ActionsPage::present_action_name_input(ProjectState& state,
                                            size_t action_index) {
  std::vector<std::string>& actions = state.actions();
  std::string& name = actions.at(action_index);

  const ImGuiStyle& style = ImGui::GetStyle();

  char buf[256];
  strncpy(buf, name.c_str(), name.length());
  buf[name.length()] = '\0';

  // Make space for action ID text and delete button.
  {
    const float id_width =
        ImGui::CalcTextSize("1 << 00").x + style.FramePadding.x * 2.f;
    const float button_width =
        ImGui::CalcTextSize(ICON_FA_TRASH).x + style.FramePadding.x * 2.f;

    const float right_items_width =
        id_width + style.ItemSpacing.x + button_width;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x -
                            right_items_width);
  }

  {
    auto s = ImGui::Scoped::ID(static_cast<int>(action_index));

    ImGui::InputText("##action_name", buf, 256,
                     // This is a hack to stop tab navigation between items,
                     // because that breaks history.
                     ImGuiInputTextFlags_AllowTabInput);
  }

  if (ImGui::IsItemActivated()) {
    m_history.start_long_edit();
  }

  if (ImGui::IsItemActive()) {
    name = buf;
    m_history.add_state(state);
  }

  if (ImGui::IsItemDeactivated()) {
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      m_history.finish_long_edit();
    } else {
      m_history.discard_long_edit();
    }
  }
}

void ActionsPage::present_action_id(size_t id, size_t num_actions) {
  auto s = ImGui::Scoped::StyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

  ImGui::Text("1 << %d", static_cast<int>(id));

  if (id < 10 && num_actions > 10) {
    ImGui::SameLine();
    ImGui::Dummy(ImGui::CalcTextSize("0"));
  }
}

void ActionsPage::present_action_delete_button(ProjectState& state,
                                               size_t action_index) {
  std::vector<std::string>& actions = state.actions();

  auto s = ImGui::Scoped::ID(static_cast<int>(action_index));

  if (ImGui::Button(ICON_FA_TRASH)) {
    for (auto& path : state.paths()) {
      for (CurvePoint& point : path.second.points()) {
        point.remove_action(action_index, true);
      }
    }

    actions.erase(actions.begin() + action_index);
    m_history.add_state(state);
  }
}

void ActionsPage::present_new_action_button(ProjectState& state) {
  std::vector<std::string>& actions = state.actions();

  auto disabled = ImGui::Scoped::Disabled(actions.size() >= 32);
  auto bold = ImGui::Scoped::Font(FontLibrary::get().bold_font);

  if (ImGui::Selectable("+ New Action", false)) {
    actions.push_back("Action");
    m_history.add_state(state);
  }
}
