#include <pages/properties.h>
#include <imgui_internal.h>
#include <pages/path_editor.h>

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
  if (_selected_pt && ImGui::CollapsingHeader("Point")) {
    PathEditorPage::CurvePointTable::iterator selected_pt = _selected_pt.value();

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

    ImGui::PushID("Weights");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, COL_WIDTH);
    ImGui::Text("Weights");
    ImGui::NextColumn();

    static float weights[2] { 0.0f, 0.0f };
    float new_weights[2] { weights[0], weights[1] };
    ImGui::DragFloat2("##Weights", new_weights, 1.0f, 0.00f, 0.0f, "%.2f");

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
      pos[0] = selected_pt->px * FIELD_X;
      pos[1] = selected_pt->py * FIELD_Y;
      rotation = selected_pt->rotation * RAD_2_DEG;
      heading = selected_pt->heading * RAD_2_DEG;
      weights[0] = selected_pt->w0;
      weights[1] = selected_pt->w1;
      adjust_angle(heading);
      adjust_angle(rotation);
    };

    if (focused) {
      if (new_pos[0] != pos[0] || new_pos[1] != pos[1]) {
        selected_pt->translate((new_pos[0] - pos[0]) / FIELD_X, (new_pos[1] - pos[1]) / FIELD_Y);
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

  if (ImGui::CollapsingHeader("Curve")) {
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
  }
  
  ImGui::End();
}

PropertiesPage PropertiesPage::instance {};
