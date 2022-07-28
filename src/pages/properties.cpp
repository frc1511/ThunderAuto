#include <pages/properties.h>
#include <imgui_internal.h>
#include <pages/path_editor.h>
#include <platform/platform.h>

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
    float new_pos[2] { pos[0], pos[1] };
    ImGui::DragFloat2("##Position", new_pos, 0.3f, 0.0f, 0.0f, "%.2f m");

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Heading ---

    ImGui::PushID("Heading");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Heading");
    ImGui::NextColumn();

    static float heading = 0.0f;
    float new_heading = heading;
    ImGui::DragFloat("##Heading", &new_heading, 1.0f, 0.0f, 0.0f, "%.2f deg");

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Rotation ---

    ImGui::PushID("Rotation");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Rotation");
    ImGui::NextColumn();

    static float rotation = 0.0f;
    float new_rotation = rotation;
    ImGui::DragFloat("##Rotation", &new_rotation, 1.0f, 0.0f, 0.0f, "%.2f deg");

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Weights ---

    ImGui::PushID("Weights");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Weights");
    ImGui::NextColumn();

    static float weights[2] { 0.0f, 0.0f };
    float new_weights[2] { weights[0], weights[1] };
    ImGui::DragFloat2("##Weights", new_weights, 1.0f, 0.00f, 0.0f, "%.2f m");

    ImGui::Columns(1);
    ImGui::PopID();

    auto adjust_angle = [](float& angle) {
      angle = std::fmod(angle, 360.0f);
      if (angle < 0.0f) {
        angle += 360.0f;
      }
    };
    
    adjust_angle(new_heading);
    adjust_angle(new_rotation);

    auto find_values = [&]() {
      pos[0] = selected_pt->px;
      pos[1] = selected_pt->py;
      rotation = selected_pt->rotation * RAD_2_DEG;
      heading = selected_pt->heading * RAD_2_DEG;
      weights[0] = selected_pt->w0;
      weights[1] = selected_pt->w1;
      adjust_angle(heading);
      adjust_angle(rotation);
    };

    if (focused) {
      if (new_pos[0] != pos[0] || new_pos[1] != pos[1]) {
        selected_pt->translate(new_pos[0] - pos[0], new_pos[1] - pos[1]);
        PathEditorPage::get()->update();
        pos[0] = new_pos[0];
        pos[1] = new_pos[1];
      }
      else if (new_heading != heading) {
        heading = new_heading;
        selected_pt->heading = heading * DEG_2_RAD;
        PathEditorPage::get()->update();
      }
      else if (new_rotation != rotation) {
        rotation = new_rotation;
        selected_pt->rotation = rotation * DEG_2_RAD;
        PathEditorPage::get()->update();
      }
      else if (new_weights[0] != weights[0] || new_weights[1] != weights[1]) {
        if (new_weights[0] < 0.03f) {
          new_weights[0] = 0.03f;
        }
        if (new_weights[1] < 0.03f) {
          new_weights[1] = 0.03f;
        }
        selected_pt->w0 = new_weights[0];
        selected_pt->w1 = new_weights[1];
        weights[0] = new_weights[0];
        weights[1] = new_weights[1];
        PathEditorPage::get()->update();
      }
      else {
        find_values();
      }
    }
    else {
      find_values();
    }
  }

  // --- Path Properties ---

  if (ImGui::CollapsingHeader("Path")) {
    // --- Export Path ---

    static char export_path_dir[256] = "${PROJECT_DIR}/${PATH_NAME}.csv";

    ImGui::PushID("Export Path");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Export Path");
    ImGui::NextColumn();

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

    static PathEditorPage::CurveStyle curve_style = PathEditorPage::CurveStyle::VELOCITY;

    const char* curve_style_names[] = {
      "Velocity",
      "Curvature",
    };

    ImGui::PushID("Curve Style");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Curve Style");
    ImGui::NextColumn();

    if (ImGui::Combo("##Curve Style", (int*)&curve_style, curve_style_names, 2)) {
      PathEditorPage::get()->set_curve_style(curve_style);
    }

    ImGui::Columns(1);
    ImGui::PopID();

    // --- Show Tangents ---

    static bool show_tangents = true;

    ImGui::PushID("Show Tangents");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Show Tangents");
    ImGui::NextColumn();

    ImGui::Checkbox("##Show Tangents", &show_tangents);

    ImGui::Columns(1);
    ImGui::PopID();

    PathEditorPage::get()->set_show_tangents(show_tangents);

    // --- Show Rotation ---

    static bool show_rotation = true;

    ImGui::PushID("Show Rotation");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Show Rotation");
    ImGui::NextColumn();

    ImGui::Checkbox("##Show Rotation", &show_rotation);

    ImGui::Columns(1);
    ImGui::PopID();

    PathEditorPage::get()->set_show_rotation(show_rotation);

    ImGui::Separator();

    // --- Curve Kind ---

    static PathEditorPage::CurveKind curve_kind = PathEditorPage::CurveKind::CUBIC_BEZIER;
    PathEditorPage::CurveKind new_curve_kind = curve_kind;

    const char* curve_kind_names[] = {
      "Cubic Bezier",
      "Cubic Hermite",
    };

    ImGui::PushID("Curve Kind");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Curve Kind");
    ImGui::NextColumn();

    ImGui::Combo("##Curve Kind", (int*)&new_curve_kind, curve_kind_names, 2);

    ImGui::Columns(1);
    ImGui::PopID();

    if (new_curve_kind != curve_kind) {
      curve_kind = new_curve_kind;
      PathEditorPage::get()->set_curve_kind(curve_kind);
      PathEditorPage::get()->update();
    }
  }
  
  ImGui::End();
}

PropertiesPage PropertiesPage::instance {};
