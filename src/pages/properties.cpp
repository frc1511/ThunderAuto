#include <ThunderAuto/pages/properties.h>
#include <imgui_internal.h>
#include <ThunderAuto/pages/path_editor.h>
#include <ThunderAuto/pages/path_manager.h>
#include <ThunderAuto/platform/platform.h>
#include <ThunderAuto/project.h>
#include <IconsFontAwesome5.h>

#define COL_WIDTH 110.0f

PropertiesPage::PropertiesPage() { }

PropertiesPage::~PropertiesPage() { }

void PropertiesPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Properties", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();

  auto adjust_angle = [](float& angle) {
    angle = std::fmod(angle, 360.0f);
    if (angle < 0.0f) {
      angle += 360.0f;
    }
  };

  std::optional<PathEditorPage::CurvePointTable::iterator> _selected_pt = PathEditorPage::get()->get_selected_point();
  
  // --- Point Properties ---

  if (_selected_pt && ImGui::CollapsingHeader(ICON_FA_CIRCLE "  Point")) {
    PathEditorPage::CurvePointTable::iterator selected_pt = _selected_pt.value();

    // --- Position ---

    ImGui::PushID("Position");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Position");
    ImGui::NextColumn();

    static float pos[2] { 0.0f, 0.0f };
    float old_pos[2] { pos[0], pos[1] };

    if (ImGui::DragFloat2("##Position", pos, 0.3f, 0.0f, 0.0f, "%.2f m")) {
      selected_pt->translate(pos[0] - old_pos[0], pos[1] - old_pos[1]);
      PathEditorPage::get()->update();
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Heading ---

    const char* headings_id = selected_pt->stop ? "Headings" : "Heading";

    ImGui::PushID(headings_id);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("%s", headings_id);
    ImGui::NextColumn();

    static float headings[2] { 0.0f, 0.0f };
    
    bool cond = false;

    std::size_t i = 0;
    if (selected_pt->end) {
      i = 1;
    }

    if (selected_pt->stop) {
      cond = ImGui::DragFloat2("##Headings", headings, 0.3f, 0.0f, 0.0f, "%.2f°");
    }
    else {
      cond = ImGui::DragFloat("##Heading", &headings[i], 1.0f, 0.0f, 0.0f, "%.2f°");
    }

    if (cond) {
      adjust_angle(headings[i]);

      selected_pt->h0 = headings[i] * DEG_2_RAD;

      if (selected_pt->stop || selected_pt->end) {
        adjust_angle(headings[!i]);
        selected_pt->h1 = (headings[!i] * DEG_2_RAD) + M_PI;
      }
      else {
        selected_pt->h1 = headings[i] * DEG_2_RAD;
      }

      PathEditorPage::get()->update();
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Rotation ---

    ImGui::PushID("Rotation");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Rotation");
    ImGui::NextColumn();

    static float rotation = 0.0f;
    if (ImGui::DragFloat("##Rotation", &rotation, 1.0f, 0.0f, 0.0f, "%.2f°")) {
      adjust_angle(rotation);
      selected_pt->rotation = rotation * DEG_2_RAD;
      PathEditorPage::get()->update();
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Weights ---

    static float weights[2] { 0.0f, 0.0f };

    if (!(selected_pt->begin && selected_pt->end)) {
      const char* weights_id = selected_pt->begin || selected_pt->end ? "Weight" : "Weights";

      ImGui::PushID(weights_id);
      ImGui::Columns(2, nullptr, false);
      ImGui::SetColumnWidth(0, COL_WIDTH);
      ImGui::Text("%s", weights_id);
      ImGui::NextColumn();

      cond = false;
      if (selected_pt->begin) {
        cond = ImGui::DragFloat("##Begin", &weights[0], 0.3f, 0.0f, 0.0f, "%.2f m");
      }
      else if (selected_pt->end) {
        cond = ImGui::DragFloat("##End", &weights[1], 0.3f, 0.0f, 0.0f, "%.2f m");
      }
      else {
        cond = ImGui::DragFloat2("##Weights", weights, 0.3f, 0.0f, 0.0f, "%.2f m");
      }

      if (cond) {
        if (weights[0] < 0.0f) {
          weights[0] = 0.0f;
        }
        if (weights[1] < 0.0f) {
          weights[1] = 0.0f;
        }
        selected_pt->w0 = weights[0];
        selected_pt->w1 = weights[1];
        PathEditorPage::get()->update();
      }

      ImGui::Columns(1);
      ImGui::PopID();
    }

    // --- Stop ---

    bool stop = selected_pt->stop;
    if (selected_pt != (PathManagerPage::get()->get_selected_path().cend() - 1) && selected_pt != PathManagerPage::get()->get_selected_path().cbegin()) {
      ImGui::PushID("Stop");
      ImGui::Columns(2, nullptr, false);
      ImGui::SetColumnWidth(0, COL_WIDTH);
      ImGui::Text("Stop");
      ImGui::NextColumn();

      if (ImGui::Checkbox("##Stop", &stop)) {
        selected_pt->stop = stop;
        selected_pt->h1 = selected_pt->h0;
        PathEditorPage::get()->update();
      }

      ImGui::Columns(1);
      ImGui::PopID();
    }

    // --- Actions ---
    if (ImGui::TreeNode("Actions")) {
      for (std::size_t i = 0; i < actions.size(); ++i) {
        std::string& name = actions.at(i);

        bool selected = selected_pt->actions & (1 << i);

        if (ImGui::Checkbox(("##action_" + std::to_string(i)).c_str(), &selected)) {
          if (selected) {
            selected_pt->actions |= (1 << i);
          }
          else {
            selected_pt->actions &= ~(1 << i);
          }
        }

        ImGui::SameLine();

        float width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - 20.0f;

        ImGui::PushItemWidth(width * 0.4f);

        char buf[256];
        strncpy(buf, name.c_str(), name.length());
        buf[name.length()] = '\0';

        if (ImGui::InputText(("##action_name_" + std::to_string(i)).c_str(), buf, 256)) {
          name = buf;
        }

        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::Text("1 << %lu", i);
        ImGui::SameLine();

        ImGui::PushID((void*)(intptr_t)i);

        if (ImGui::Button(ICON_FA_TRASH)) {
          unsigned before_actions = selected_pt->actions;
          selected_pt->actions = 0;
          std::size_t l = 0;
          for (std::size_t j = 0; j < actions.size(); ++j) {
            if (i == j) {
              continue;
            }

            if (before_actions & (1 << j)) {
              selected_pt->actions |= (1 << l);
            }
            l++;
          }

          actions.erase(actions.begin() + i);
        }

        ImGui::PopID();
      }

      bool add_action_disabled = actions.size() >= 32;

      if (add_action_disabled) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      }

      if (ImGui::Selectable("+ New Action", false)) {
        actions.push_back("Action");
      }

      if (add_action_disabled) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
      }

      ImGui::TreePop();
    }

    // Update values from the path editor.
    pos[0] = selected_pt->px;
    pos[1] = selected_pt->py;
    stop = selected_pt->stop;
    headings[0] = selected_pt->h0 * RAD_2_DEG;
    headings[1] = (selected_pt->h1 - M_PI * (stop || selected_pt->end)) * RAD_2_DEG;
    rotation = selected_pt->rotation * RAD_2_DEG;
    weights[0] = selected_pt->w0;
    weights[1] = selected_pt->w1;
    adjust_angle(headings[0]);
    adjust_angle(headings[1]);
    adjust_angle(rotation);
  }

  // --- Path Properties ---

  if (ImGui::CollapsingHeader(ICON_FA_BEZIER_CURVE "  Path")) {
    // --- Export Button ---

    ImGui::PushID("Export");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Export to CSV");
    ImGui::NextColumn();

    if (ImGui::Button("Export")) {
      PathEditorPage::get()->export_path();
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Auto Export ---

    /*
    ImGui::PushID("Auto Export");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Auto Export");
    ImGui::NextColumn();

    static bool auto_export = false;
    if (ImGui::Checkbox("##Auto Export", &auto_export)) {
      if (auto_export) {
        PathEditorPage::get()->export_path();
      }
    }

    ImGui::Columns(1);
    ImGui::PopID();
    */

    ImGui::Separator();

    // --- Curve Overlay ---

    ImGui::PushID("Curve Overlay");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Curve Overlay");
    ImGui::NextColumn();

    static PathEditorPage::CurveOverlay curve_overlay = PathEditorPage::CurveOverlay::VELOCITY;

    const char* curve_overlay_names[] = {
      "Velocity",
      "Curvature",
    };

    if (ImGui::Combo("##Curve Overlay", (int*)&curve_overlay, curve_overlay_names, 2)) {
      PathEditorPage::get()->set_curve_overlay(curve_overlay);
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Show Tangents ---

    ImGui::PushID("Show Tangents");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Show Tangents");
    ImGui::NextColumn();

    static bool show_tangents = true;

    if (ImGui::Checkbox("##Show Tangents", &show_tangents)) {
      PathEditorPage::get()->set_show_tangents(show_tangents);
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Show Rotation ---

    ImGui::PushID("Show Rotation");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Show Rotation");
    ImGui::NextColumn();

    static bool show_rotation = true;

    if (ImGui::Checkbox("##Show Rotation", &show_rotation)) {
      PathEditorPage::get()->set_show_rotation(show_rotation);
    }

    ImGui::Columns(1);
    ImGui::PopID();
  }
  
  ImGui::End();
}

void PropertiesPage::set_project(Project* _project) {
  project = _project;
}

PropertiesPage PropertiesPage::instance {};
