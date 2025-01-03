#include <ThunderAuto/pages/path_manager_page.h>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.h>

void PathManagerPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(name(), running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }

  ProjectState state = m_history.current_state();

  static bool input_active = false;
  static bool was_input_active = false;

  char buf[256] = "";
  size_t num_paths = state.paths().size();
  bool done = false;
  for (std::size_t i = 0; i < num_paths && !done; ++i) {
    std::string& name = state.paths().at(i).first;

    strncpy(buf, name.c_str(), 255);
    buf[name.length()] = 0;

    bool is_selected = i == state.current_path_index();

    ImGui::PushID(i);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 40.0f);

    bool tmp_input_active = is_selected ? input_active : false;
    bool tmp_input_was_active = is_selected ? was_input_active : false;

    if (selectable_input(name.c_str(), is_selected, buf, 255,
                         tmp_input_active)) {
      if (!is_selected) {
        state.current_path_index() = i;
        state.selected_point_index() = -1;
        m_history.add_state(state);
        state.current_path().output(m_cached_curve,
                                    preview_output_curve_settings);
        is_selected = true;
      }
    }
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem(ICON_FA_COPY "  Duplicate")) {
        duplicate_path(state, i);
        done = true;
      }

      {
        ImGuiScopedDisabled disabled(num_paths <= 1);

        if (ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete")) {
          delete_path(state, i);
          done = true;
        }
      }

      ImGui::EndPopup();
    }

    if (!tmp_input_active && tmp_input_was_active) {
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        name = buf;
        m_history.add_state(state);
      }
    }

    if (is_selected) {
      input_active = tmp_input_active;
      was_input_active = tmp_input_active;
    }

    ImGui::NextColumn();

    // Delete button.
    if (state.paths().size() != 1) {
      if (ImGui::Button(ICON_FA_TRASH_ALT)) {
        delete_path(state, i);
        done = true;
      }
    }

    ImGui::Columns(1);
    ImGui::PopID();
  }

  if (ImGui::Selectable("+ New Path", false)) {
    state.paths().emplace_back("new_path", default_new_curve);
    m_history.add_state(state);
  }

  ImGui::End();
}

bool PathManagerPage::selectable_input(const char* label, bool selected,
                                       char* buf, std::size_t buf_size,
                                       bool& input_active) {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  ImVec2 pos_before = window->DC.CursorPos;

  ImGui::PushID(label);
  bool ret = ImGui::Selectable("##Selectable", selected,
                               ImGuiSelectableFlags_AllowDoubleClick |
                                   ImGuiSelectableFlags_AllowItemOverlap,
                               ImVec2(0.0f, 20.0f));

  const ImGuiID id = window->GetID("##Input");
  const bool input_start =
      ret ? ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) : false;

  if (input_start) {
    ImGui::SetActiveID(id, window);
    input_active = true;
  }

  if (input_start || input_active) {
    ImVec2 pos_after = window->DC.CursorPos;
    window->DC.CursorPos = pos_before;

    // Good characters for a filename.
    const auto text_validate = [](ImGuiInputTextCallbackData* data) -> int {
      return std::isalnum(data->EventChar) || data->EventChar == '_' ? 0 : 1;
    };

    ImGui::InputText("##Input", buf, buf_size,
                     ImGuiInputTextFlags_CallbackCharFilter |
                         ImGuiInputTextFlags_AllowTabInput,
                     text_validate);
    window->DC.CursorPos = pos_after;

    if (ImGui::IsItemDeactivated()) {
      input_active = false;
    }
  } else {
    window->DrawList->AddText(pos_before, ImGui::GetColorU32(ImGuiCol_Text),
                              buf);
  }

  ImGui::PopID();
  return ret;
}

void PathManagerPage::duplicate_path(ProjectState& state, std::size_t index) {
  const std::string& name = state.paths().at(index).first;
  const Curve& curve = state.paths().at(index).second;

  state.paths().emplace_back(name + " copy", curve);
  m_history.add_state(state);

  state.current_path().output(m_cached_curve, preview_output_curve_settings);
}

void PathManagerPage::delete_path(ProjectState& state, std::size_t index) {
  if ((state.current_path_index() == 1 && index == 0) ||
      state.current_path_index() == state.paths().size() - 1) {
    state.current_path_index() -= 1;
  }

  state.paths().erase(state.paths().cbegin() + index);
  m_history.add_state(state);

  state.current_path().output(m_cached_curve, preview_output_curve_settings);
}

