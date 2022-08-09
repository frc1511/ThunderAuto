#include <pages/path_manager.h>
#include <pages/path_editor.h>
#include <project.h>
#include <IconsFontAwesome5.h>
#include <imgui_internal.h>

static bool selectable_input(const char* label, bool selected, char* buf, std::size_t buf_size, bool& input_active, bool& input_was_active) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 pos_before = window->DC.CursorPos;

    ImGui::PushID(label);
    bool ret = ImGui::Selectable("##Selectable", selected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, 20.0f));

    ImGuiID id = window->GetID("##Input");
    bool input_start = ret ? ImGui::IsMouseDoubleClicked(0) : false;

    if (input_start) {
      ImGui::SetActiveID(id, window);
      input_active = true;
      input_was_active = false;
    }

    if (input_active || input_start) {
      ImVec2 pos_after = window->DC.CursorPos;
      window->DC.CursorPos = pos_before;

      ImVec2 max(ImGui::GetItemRectMax());
      ImRect lastItemRect(ImGui::GetItemRectMin(), ImVec2(max.x + 100.0f, max.y));
      ret = ImGui::InputText("##Input", buf, buf_size, ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData* data) -> int {
        return std::isalnum(data->EventChar) || data->EventChar == '_' ? 0 : 1;
      });
      window->DC.CursorPos = pos_after;

      ImGuiIO& io = ImGui::GetIO();
      if (io.WantTextInput) {
        input_was_active = true;
      }
      else if (input_was_active) {
        input_active = false;
      }
    }
    else {
      window->DrawList->AddText(pos_before, ImGui::GetColorU32(ImGuiCol_Text), buf);
    }

    ImGui::PopID();
    return ret;
}

PathManagerPage::PathManagerPage() { }

PathManagerPage::~PathManagerPage() { }

void PathManagerPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Selector", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();

  static bool input_active = false, input_was_active = false;

  for (std::size_t i = 0; i < project->paths.size(); ++i) {
    std::string& name = project->paths.at(i).first;

    std::size_t prev_selected = selected;

    char buf[256];
    strncpy(buf, name.c_str(), name.length());
    buf[name.length()] = '\0';

    ImGui::PushID((void*)(intptr_t)i);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 40.0f);

    bool tmp_input_active = (i == selected) ? input_active : false;
    bool tmp_input_was_active = (i == selected) ? input_was_active : false;

    if (selectable_input(("##path_" + std::to_string(i)).c_str(), i == selected, buf, 256, tmp_input_active, tmp_input_was_active)) {
      selected = i;
      name = buf;
    }

    if (i == selected) {
      input_active = tmp_input_active;
      input_was_active = tmp_input_was_active;
    }

    ImGui::NextColumn();

    // Delete button
    if (project->paths.size() != 1) {
      if (ImGui::Button(ICON_FA_TRASH)) {
        if (selected == 1 && i == 0) {
          selected = 0;
        }
        if (selected == project->paths.size() - 1) {
          selected = selected - 1;
        }
        
        project->paths.erase(project->paths.begin() + i);

        PathEditorPage::get()->update();
        PathEditorPage::get()->reset_selected_point();
      }
    }

    if (prev_selected != selected) {
      PathEditorPage::get()->update();
      PathEditorPage::get()->reset_selected_point();
    }

    ImGui::Columns(1);
    ImGui::PopID();
  }

  if (ImGui::Selectable("+ New Path", false)) {
    project->paths.emplace_back("the_path", PathEditorPage::CurvePointTable({
      { 8.124f, 1.78f, 4.73853f, 4.73853f, 1.44372f, 1.70807f, 4.73853, false, true, false, 0 },
      { 4.0f,   1.5f,  2.0944f,  2.0944f,  2.0f,     2.0f,     2.0944,  false, false, true, 0 },
    }));
  }

  ImGui::End();
}

void PathManagerPage::set_project(Project* _project) {
  project = _project;
}

PathEditorPage::CurvePointTable& PathManagerPage::get_selected_path() const {
  return project->paths.at(selected).second;
}

const std::string& PathManagerPage::get_selected_path_name() const {
  return project->paths.at(selected).first;
}

PathManagerPage PathManagerPage::instance {};
