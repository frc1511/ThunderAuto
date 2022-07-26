#include <popups/new_project.h>
#include <thunder_auto.h>
#include <imgui_internal.h>
#include <popups/new_field.h>

NewProjectPopup::NewProjectPopup() { }

const Field field_constants[] {
  { "field_2022.png", ImVec2(0.12f, 0.16f), ImVec2(0.88f, 0.84f) },
};

NewProjectPopup::~NewProjectPopup() { }

void NewProjectPopup::present(bool* running) {
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }
  
  // --- File ---

  static char deploy_path_buf[256] = "";
  
  ImGui::InputText("Path", deploy_path_buf, 256, ImGuiInputTextFlags_None);
  std::string deploy_path = deploy_path_buf;
  
  ImGui::SameLine();
  
  if (ImGui::Button("Browse")) {
    deploy_path = Platform::get_current()->save_file_dialog(FILE_EXTENSION);
    strncpy(deploy_path_buf, deploy_path.c_str(), deploy_path.length());
  }
  
  bool has_deploy_path = !deploy_path.empty();

  // --- Field Selection ---

  static std::optional<Field> field;

  const char* fields[] = { "2022 - Rapid React", "Custom" };
  static int current_field = 0;

  const char* field_str = (current_field == 1 && field) ? field->img_path.c_str() : fields[current_field];

  if (current_field == 1) {
    field = NewFieldPopup::get()->get_field();
  }
  else {
    field = field_constants[current_field];
  }

  if (ImGui::BeginCombo("Field", field_str)) {
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
  
  ImGui::Separator();

  // --- Drive Controller Selection ---
  
  const char* controllers[] = { "Ramsete", "Holonomic" };
  static int current_controller = 0;
  
  if (ImGui::BeginCombo("Controller Type", controllers[current_controller])) {
    for (int i = 0; i < 2; i++) {
      if (ImGui::Selectable(controllers[i], current_controller == i)) {
        current_controller = i;
      }
    }
    ImGui::EndCombo();
  }

  // --- Max Acceleration ---
  
  static char accel_buf[10] = "";
  
  ImGui::InputText("Max Acceleration (m/s^2)", accel_buf, 9, ImGuiInputTextFlags_CharsDecimal);
  double max_accel = std::atof(accel_buf);

  // --- Max Deceleration ---
  
  static char decel_buf[10] = "";
  
  ImGui::InputText("Max Deceleration (m/s^2)", decel_buf, 9, ImGuiInputTextFlags_CharsDecimal);
  double max_decel = std::atof(decel_buf);

  // --- Max Velocity ---
  
  static char vel_buf[10] = "";
  
  ImGui::InputText("Max Velocity (m/s)", vel_buf, 9, ImGuiInputTextFlags_CharsDecimal);
  double max_vel = std::atof(vel_buf);

  // --- Create and Cancel Buttons ---
  
  bool create_disabled = false;
  
  static const char* err_text = "";
  if (!has_deploy_path) {
    err_text = "Project path required";
    create_disabled = true;
  }
  else if (max_accel <= 0) {
    err_text = "Max acceleration should be greater than 0 m/s^2";
    create_disabled = true;
  }
  else if (max_decel >= 0) {
    err_text = "Max declaration should be less than 0 m/s^2";
    create_disabled = true;
  }
  else if (max_vel <= 0) {
    err_text = "Max velocity should be greater than 0 m/s";
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
    
    project = { deploy_path, field.value(), drivetrain, atof(accel_buf), atof(decel_buf), atof(vel_buf) };
    
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

