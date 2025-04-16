#include <ThunderAuto/pages/path_manager_page.hpp>

#include <ThunderAuto/font_library.hpp>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.hpp>

static const size_t MAX_NAME_LENGTH = 256;

void PathManagerPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
  ImGui::Scoped s = ImGui::Scoped::Window(name(), running);
  if (!s)
    return;

  ProjectState state = m_history.current_state();

  char buf[MAX_NAME_LENGTH] = "";
  size_t num_paths = state.paths().size();
  bool done = false;
  for (size_t i = 0; i < num_paths && !done; ++i) {
    auto id = ImGui::Scoped::ID(static_cast<int>(i));

    std::string& name = state.paths().at(i).first;
    strncpy(buf, name.c_str(), MAX_NAME_LENGTH - 1);
    buf[min(name.length(), MAX_NAME_LENGTH - 1)] = '\0';

    bool is_selected = (i == state.current_path_index());

    bool input_was_active = is_selected ? m_was_input_active : false;
    bool input_active = input_was_active;

    if (selectable_input(name.c_str(), is_selected, buf, MAX_NAME_LENGTH,
                         input_active)) {
      if (!is_selected) {
        state.current_path_index() = i;
        state.selected_point_index() = -1;
        m_history.add_state(state, false);
        state.current_path().output(m_cached_curve,
                                    preview_output_curve_settings);
        is_selected = true;
      }
    }
    if (input_active) {
      if (!input_was_active) {
        m_history.start_long_edit();
      }
    } else {
      if (input_was_active) {
        if (ImGui::IsItemDeactivatedAfterEdit()) {
          name = buf;
          m_history.add_state(state);
          m_history.finish_long_edit();
        } else {
          m_history.discard_long_edit();
        }
      }

      if (auto popup = ImGui::Scoped::PopupContextItem()) {
        done = present_context_menu(state, i);
      }
    }

    if (is_selected) {
      m_was_input_active = input_active;
    }
  }

  present_new_path_button(state);
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

void PathManagerPage::reverse_path(ProjectState& state, std::size_t index) {
  Curve& curve = state.paths().at(index).second;
  std::reverse(curve.points().begin(), curve.points().end());

  for (CurvePoint& pt : curve.points()) {
    HeadingAngles headings = pt.headings();
    std::swap(headings.incoming, headings.outgoing);
    pt.set_headings(headings);

    HeadingWeights weights = pt.heading_weights();
    std::swap(weights.incoming, weights.outgoing);
    pt.set_heading_weights(weights);
  }

  m_history.add_state(state);
  state.current_path().output(m_cached_curve, preview_output_curve_settings);
}

bool PathManagerPage::present_context_menu(ProjectState& state, size_t index) {
  if (ImGui::MenuItem("\xef\x8d\xa3"
                      "  Reverse Direction")) {
    reverse_path(state, index);
    return true;
  }

  if (ImGui::MenuItem(ICON_FA_COPY "  Duplicate")) {
    duplicate_path(state, index);
    return true;
  }

  {
    ImGuiScopedDisabled disabled(state.paths().size() < 2);

    if (ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete")) {
      delete_path(state, index);
      return true;
    }
  }

  return false;
}

void PathManagerPage::present_new_path_button(ProjectState& state) {
  auto bold = ImGui::Scoped::Font(FontLibrary::get().bold_font);

  if (ImGui::Selectable("+ New Path", false)) {
    state.paths().emplace_back("new_path", default_new_curve);
    m_history.add_state(state);
  }
}

bool PathManagerPage::selectable_input(const char* label,
                                       bool selected,
                                       char* buf,
                                       std::size_t buf_size,
                                       bool& input_active) {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  ImVec2 pos_before = window->DC.CursorPos;

  const ImGuiStyle& style = ImGui::GetStyle();

  auto s = ImGui::Scoped::ID(label);

  bool ret;
  {
    const ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowDoubleClick |
                                       ImGuiSelectableFlags_AllowOverlap;

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const ImVec2 frame_size =
        ImVec2(0.f, label_size.y + style.FramePadding.y * 2.0f);

    ret = ImGui::Selectable("##Selectable", selected, flags, frame_size);
  }

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

    const ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackCharFilter |
                                      ImGuiInputTextFlags_AllowTabInput;

    const auto text_validate = [](ImGuiInputTextCallbackData* data) -> int {
      // Good characters for a filename.
      return std::isalnum(data->EventChar) || data->EventChar == '_' ? 0 : 1;
    };

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##Input", buf, buf_size, flags, text_validate);
    window->DC.CursorPos = pos_after;

    if (!input_start && ImGui::IsItemDeactivated()) {
      input_active = false;
    }
  } else {
    window->DrawList->AddText(pos_before, ImGui::GetColorU32(ImGuiCol_Text),
                              buf);
  }

  return ret;
}
