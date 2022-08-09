#include <pages/settings.h>
#include <project.h>
#include <pages/path_editor.h>
#include <IconsFontAwesome5.h>

#define COL_WIDTH 150.0f

SettingsPage::SettingsPage() { }

SettingsPage::~SettingsPage() { }

void SettingsPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Project Settings", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();

  // --- Drive Controller Selection ---

  ImGui::PushID("Controller Type");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_CAR "  Controller Type");
  ImGui::NextColumn();
  
  const char* controllers[] = { "Holonomic", "Ramsete" };
  
  if (ImGui::BeginCombo("##Controller Type", controllers[controller])) {
    for (int i = 0; i < 2; i++) {
      if (ImGui::Selectable(controllers[i], controller == i, ImGuiSelectableFlags_Disabled * (i == 1))) {
        controller = i;
      }
    }
    ImGui::EndCombo();
  }

  ImGui::Columns(1);
  ImGui::PopID();

  // --- Max Acceleration ---

  ImGui::PushID("Max Acceleration");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_SKIING "  Max Acceleration");
  ImGui::NextColumn();

  ImGui::DragFloat("##Max Acceleration", &max_accel, 0.1f, 0.0f, 0.0f, "%.1f m/s²");

  if (max_accel < 0.0f) {
    max_accel = 0.0f;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  // --- Max Velocity ---

  ImGui::PushID("Max Velocity");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_SKIING_NORDIC "  Max Velocity");
  ImGui::NextColumn();
  
  ImGui::DragFloat("##Max Velocity", &max_vel, 0.1f, 0.0f, 0.0f, "%.2f m/s");

  if (max_vel < 0.0f) {
    max_vel = 0.0f;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  ImGui::Separator();

  // --- Robot Length ---

  ImGui::PushID("Robot Length");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_RULER_VERTICAL "   Robot Length");
  ImGui::NextColumn();

  ImGui::DragFloat("##Robot Length", &robot_length, 0.1f, 0.0f, 0.0f, "%.2f m");

  if (robot_length < 0.0f) {
    robot_length = 0.0f;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  // --- Robot Width ---

  ImGui::PushID("Robot Width");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_RULER_HORIZONTAL "  Robot Width");
  ImGui::NextColumn();

  ImGui::DragFloat("##Robot Width", &robot_width, 0.1f, 0.0f, 0.0f, "%.2f m");

  if (robot_width < 0.0f) {
    robot_width = 0.0f;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  // --- Update button ---
  
  bool update_disabled = false;
  
  static const char* err_text = "";
  if (!max_accel) {
    err_text = "Max acceleration can not be 0 m/s^2";
    update_disabled = true;
  }
  else if (!max_vel) {
    err_text = "Max velocity can not be 0 m/s";
    update_disabled = true;
  }
  else if (!robot_length) {
    err_text = "Robot length can not be 0 m";
    update_disabled = true;
  }
  else if (!robot_width) {
    err_text = "Robot width can not be 0 m";
    update_disabled = true;
  }

  if (update_disabled) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  if (ImGui::Button("Update")) {
    project->settings.drive_ctrl = static_cast<DriveController>(controller);
    project->settings.max_accel = max_accel;
    project->settings.max_vel = max_vel;
    project->settings.robot_length = robot_length;
    project->settings.robot_width = robot_width;

    PathEditorPage::get()->update();
    ProjectManager::get()->save_project();
  }
  if (update_disabled) {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
      ImGui::SetTooltip("* %s", err_text);
    }
  }

  ImGui::End();
}

void SettingsPage::set_project(Project* _project) {
  project = _project;
}

void SettingsPage::reset() {
  controller = static_cast<int>(project->settings.drive_ctrl); 
  max_accel = project->settings.max_accel;
  max_vel = project->settings.max_vel;
  robot_length = project->settings.robot_length;
  robot_width = project->settings.robot_width;
}

SettingsPage SettingsPage::instance;
