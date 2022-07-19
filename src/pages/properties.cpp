#include <pages/properties.h>
#include <imgui_internal.h>
#include <cmath>
#include <string>
#include <pages/path_editor.h>

PropertiesPage::PropertiesPage() { }

PropertiesPage::~PropertiesPage() { }

void PropertiesPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Properties", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();
  ImGui::Text("%d\n", focused);

  std::optional<PathEditorPage::CurvePointTable::iterator> _selected_pt = PathEditorPage::get()->get_selected_point();
  if (!_selected_pt) {
    ImGui::End();
    return;
  }

  PathEditorPage::CurvePointTable::iterator selected_pt = _selected_pt.value();

  ImGui::PushID("Position");
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 100.0f);
  ImGui::Text("Position");
  ImGui::NextColumn();

  static float pos[2] { 0.0f, 0.0f };
  float new_pos[2] { pos[0], pos[1] };
  ImGui::DragFloat2("##Position", new_pos, 0.01f, 0.0f, 0.0f, "%.2f m");

  ImGui::Columns(1);
  ImGui::PopID();

  ImGui::PushID("Heading");
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 100.0f);
  ImGui::Text("Heading");
  ImGui::NextColumn();

  static float heading = 0.0f;
  float new_heading = heading;
  ImGui::DragFloat("##Heading", &new_heading, -1.0f, 0.0f, 0.0f, "%.2f deg");

  ImGui::Columns(1);
  ImGui::PopID();

  ImGui::PushID("Rotation");
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 100.0f);
  ImGui::Text("Rotation");
  ImGui::NextColumn();

  static float rotation = 0.0f;
  float new_rotation = rotation;
  ImGui::DragFloat("##Rotation", &new_rotation, -1.0f, 0.0f, 0.0f, "%.2f deg");

  ImGui::Columns(1);
  ImGui::PopID();

  ImGui::PushID("Weights");
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 100.0f);
  ImGui::Text("Weights");
  ImGui::NextColumn();

  static float weights[2] { 0.0f, 0.0f };
  float new_weights[2] { weights[0], weights[1] };
  ImGui::DragFloat2("##Weights", new_weights, 0.01f, 0.00f, 0.0f, "%.2f m");

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

  if (focused) {
    if (new_pos[0] != pos[0] || new_pos[1] != pos[1]) {
      selected_pt->translate(new_pos[0] - pos[0], new_pos[1] - pos[1]);
      PathEditorPage::get()->update();
      pos[0] = new_pos[0];
      pos[1] = new_pos[1];
    }

    if (new_heading != heading) {
      heading = new_heading;
      selected_pt->heading = heading * (M_PI / 180.0f);
      PathEditorPage::get()->update();
    }
    if (new_rotation != rotation) {
      rotation = new_rotation;
      selected_pt->rotation = rotation * (M_PI / 180.0f);
      PathEditorPage::get()->update();
    }
    if (new_weights[0] != weights[0] || new_weights[1] != weights[1]) {
      if (new_weights[0] < 0.03f) {
        new_weights[0] = 0.03f;
      }
      if (new_weights[1] < 0.03f) {
        new_weights[1] = 0.03f;
      }
      selected_pt->w0 = new_weights[0];
      selected_pt->w1 = new_weights[1];
      PathEditorPage::get()->update();
      weights[0] = new_weights[0];
      weights[1] = new_weights[1];
    }
  }
  else {
    pos[0] = selected_pt->px;
    pos[1] = selected_pt->py;
    rotation = selected_pt->rotation * 180.0f / M_PI;
    heading = selected_pt->heading * 180.0f / M_PI;
    weights[0] = selected_pt->w0;
    weights[1] = selected_pt->w1;
  }
  
  ImGui::End();
}

PropertiesPage PropertiesPage::instance {};
