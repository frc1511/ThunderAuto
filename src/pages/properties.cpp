#include <pages/properties.h>
#include <imgui_internal.h>
#include <pages/path_editor.h>
#include <platform/platform.h>
#include <project.h>

#define COL_WIDTH 100.0f

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

  if (_selected_pt && ImGui::CollapsingHeader("Point")) {
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

    ImGui::PushID("Heading");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Heading");
    ImGui::NextColumn();

    static float heading = 0.0f;
    if (ImGui::DragFloat("##Heading", &heading, 1.0f, 0.0f, 0.0f, "%.2f deg")) {
      adjust_angle(heading);
      selected_pt->heading = heading * DEG_2_RAD;
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
    if (ImGui::DragFloat("##Rotation", &rotation, 1.0f, 0.0f, 0.0f, "%.2f deg")) {
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

      bool cond = false;
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

    if (selected_pt != (project->points.cend() - 1) && selected_pt != project->points.cbegin()) {
      ImGui::PushID("Stop");
      ImGui::Columns(2, nullptr, false);
      ImGui::SetColumnWidth(0, COL_WIDTH);
      ImGui::Text("Stop");
      ImGui::NextColumn();

      bool stop = selected_pt->stop;
      if (ImGui::Checkbox("##Stop", &stop)) {
        selected_pt->stop = stop;
        PathEditorPage::get()->update();
      }

      stop = selected_pt->stop;

      ImGui::Columns(1);
      ImGui::PopID();
    }

    // Update values from the path editor.
    pos[0] = selected_pt->px;
    pos[1] = selected_pt->py;
    rotation = selected_pt->rotation * RAD_2_DEG;
    heading = selected_pt->heading * RAD_2_DEG;
    weights[0] = selected_pt->w0;
    weights[1] = selected_pt->w1;
    adjust_angle(heading);
    adjust_angle(rotation);
  }

  // --- Path Properties ---

  if (ImGui::CollapsingHeader("Path")) {
    // --- Export Path ---

    ImGui::PushID("Export Path");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Export Path");
    ImGui::NextColumn();

    static char export_path_dir[256] = "${PROJECT_DIR}/${PATH_NAME}.csv";

    ImGui::InputText("##Export Path", export_path_dir, 256);

    ImGui::SameLine();

    std::string path_dir;
    if (ImGui::Button("Browse")) {
      path_dir = Platform::get_current()->save_file_dialog("csv");
      if (!path_dir.empty()) {
        memset(export_path_dir, 0, 256);
        strncpy(export_path_dir, path_dir.c_str(), path_dir.length());
      }
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Auto Export ---

    ImGui::PushID("Auto Export");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Auto Export");
    ImGui::NextColumn();

    static bool auto_export = false;
    if (ImGui::Checkbox("##Auto Export", &auto_export)) {
      if (auto_export) {
        PathEditorPage::get()->export_path(export_path_dir);
      }
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Export Button ---

    if (auto_export) {
      ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    if (ImGui::Button("Export")) {
      PathEditorPage::get()->export_path(export_path_dir);
    }

    if (auto_export) {
      ImGui::PopItemFlag();
      ImGui::PopStyleVar();
    }

    ImGui::Separator();

    // --- Curve Style ---

    ImGui::PushID("Curve Style");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Curve Style");
    ImGui::NextColumn();

    static PathEditorPage::CurveStyle curve_style = PathEditorPage::CurveStyle::VELOCITY;

    const char* curve_style_names[] = {
      "Velocity",
      "Curvature",
    };

    if (ImGui::Combo("##Curve Style", (int*)&curve_style, curve_style_names, 2)) {
      PathEditorPage::get()->set_curve_style(curve_style);
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
