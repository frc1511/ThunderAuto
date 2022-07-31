#include <popups/new_project.h>
#include <thunder_auto.h>
#include <imgui_internal.h>
#include <popups/new_field.h>

#define COL_WIDTH 100.0f

NewProjectPopup::NewProjectPopup() { }

const Field field_constants[] {
  Field(Field::BuiltinImage::FIELD_2022, ImVec2(0.12f, 0.16f), ImVec2(0.88f, 0.84f)),
};

NewProjectPopup::~NewProjectPopup() { }

void NewProjectPopup::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(370, 250), ImGuiCond_FirstUseEver);
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_None)) {
    return;
  }
  
  // --- File ---

  ImGui::PushID("Path");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text("Path");
  ImGui::NextColumn();

  static char deploy_path_buf[256] = "";
  
  ImGui::InputText("##Path", deploy_path_buf, 256, ImGuiInputTextFlags_None);
  std::string deploy_path = deploy_path_buf;
  
  ImGui::SameLine();
  
  if (ImGui::Button("Browse")) {
    deploy_path = Platform::get_current()->save_file_dialog(FILE_EXTENSION);
    strncpy(deploy_path_buf, deploy_path.c_str(), deploy_path.length());
  }
  
  bool has_deploy_path = !deploy_path.empty();

  ImGui::Columns(1);
  ImGui::PopID();

  // --- Field Selection ---

  ImGui::PushID("Field");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text("Field");
  ImGui::NextColumn();

  static std::optional<Field> field;

  const char* fields[] = { "2022 - Rapid React", "Custom" };
  static int current_field = 0;

  if (current_field == 1) {
    field = NewFieldPopup::get()->get_field();
    if (!field) {
      current_field = 0;
    }
  }
  if (current_field == 0) {
    field = field_constants[current_field];
  }

  const char* field_str = current_field == 1 ? std::get<std::filesystem::path>(field->img).c_str() : fields[current_field];

  if (ImGui::BeginCombo("##Field", field_str)) {
    for (int i = 0; i < 2; i++) {
      if (ImGui::Selectable(fields[i], current_field == i)) {
        current_field = i;
        if (current_field == 1) {
          show_new_field_popup = true;
        }
      }
    }
    ImGui::EndCombo();
  }

  ImGui::Columns(1);
  ImGui::PopID();
  
  ImGui::Separator();

  // --- Drive Controller Selection ---

  ImGui::PushID("Controller Type");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text("Controller Type");
  ImGui::NextColumn();
  
  const char* controllers[] = { "Holonomic", "Ramsete" };
  static int current_controller = 0;
  
  if (ImGui::BeginCombo("##Controller Type", controllers[current_controller])) {
    for (int i = 0; i < 2; i++) {
      if (ImGui::Selectable(controllers[i], current_controller == i, ImGuiSelectableFlags_Disabled * (i == 1))) {
        current_controller = i;
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
  ImGui::Text("Max Acceleration");
  ImGui::NextColumn();

  static float max_accel = 3.0f;
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
  ImGui::Text("Max Velocity");
  ImGui::NextColumn();
  
  static float max_vel = 4.0f;
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
  ImGui::Text("Robot Length");
  ImGui::NextColumn();

  static float robot_length = 0.80645f;
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
  ImGui::Text("Robot Width");
  ImGui::NextColumn();

  static float robot_width = 0.7112f;
  ImGui::DragFloat("##Robot Width", &robot_width, 0.1f, 0.0f, 0.0f, "%.2f m");

  if (robot_width < 0.0f) {
    robot_width = 0.0f;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  // --- Create and Cancel Buttons ---
  
  bool create_disabled = false;
  
  static const char* err_text = "";
  if (!has_deploy_path) {
    err_text = "Project path required";
    create_disabled = true;
  }
  else if (!max_accel) {
    err_text = "Max acceleration can not be 0 m/s^2";
    create_disabled = true;
  }
  else if (!max_vel) {
    err_text = "Max velocity can not be 0 m/s";
    create_disabled = true;
  }
  else if (!robot_length) {
    err_text = "Robot length can not be 0 m";
    create_disabled = true;
  }
  else if (!robot_width) {
    err_text = "Robot width can not be 0 m";
    create_disabled = true;
  }

  // --- Cancel Button ---
  
  if (ImGui::Button("Cancel")) {
    has_project = false;
    goto close;
  }
  
  ImGui::SameLine();

  // --- Create Button ---

  if (create_disabled) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  if (ImGui::Button("Create")) {
    has_project = true;
    
    DriveController drivetrain = current_controller == 0 ? DriveController::RAMSETE : DriveController::HOLONOMIC;
    
    project = { deploy_path, field.value(), drivetrain, max_accel, max_vel, robot_length, robot_width };
    
    goto close;
  }
  if (create_disabled) {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
    
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
      ImGui::SetTooltip("* %s", err_text);
    }
  }
  
  ImGui::EndPopup();
  
  return;
  
close:
  ImGui::CloseCurrentPopup();
  *running = false;
  ImGui::EndPopup();
}

NewProjectPopup NewProjectPopup::instance {};

