#include <popups/new_project.h>
#include <imgui_internal.h>

#ifdef THUNDER_AUTO_MACOS
# include <platform/macos/macos.h>
#elif THUNDER_AUTO_WINDOWS
# include <platform/windows/windows.h>
#elif THUNDER_AUTO_LINUX
# include <platform/linux/linux.h>
#endif

NewProjectPopup::NewProjectPopup() {
#ifdef THUNDER_AUTO_MACOS
  platform = PlatformMacOS::get();
#elif THUNDER_AUTO_WINDOWS
  platform = PlatformWindows::get();
#elif THUNDER_AUTO_LINUX
  platform = PlatformLinux::get();
#endif
}

NewProjectPopup::~NewProjectPopup() { }

void NewProjectPopup::present(bool* running) {
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }
  static char path_buf[256] = "";
  
  ImGui::InputText("File Path", path_buf, 256, ImGuiInputTextFlags_None);
  path = path_buf;
  
  ImGui::SameLine();
  
  if (ImGui::Button("Browse")) {
    path = platform->save_file_dialog();
    strncpy(path_buf, path.c_str(), path.length());
  }
  
  bool has_path = !path.empty();
  
  const char* controllers[] = { "Ramsete (ex. tank)", "Holonomic (ex. swerve)" };
  static int current_controller = 0;
  
  if (ImGui::BeginCombo("Controller Type", controllers[current_controller])) {
    for (int i = 0; i < 2; i++) {
      if (ImGui::Selectable(controllers[i], current_controller == i)) {
        current_controller = i;
      }
    }
    ImGui::EndCombo();
  }
  
  ImGui::Separator();
  
  static char accel_buf[10] = "";
  
  ImGui::InputText("Max Acceleration (m/s^2)", accel_buf, 9, ImGuiInputTextFlags_CharsDecimal);
  double max_accel = atof(accel_buf);
  
  static char decel_buf[10] = "";
  
  ImGui::InputText("Max Deceleration (m/s^2)", decel_buf, 9, ImGuiInputTextFlags_CharsDecimal);
  double max_decel = atof(decel_buf);
  
  static char vel_buf[10] = "";
  
  ImGui::InputText("Max Velocity (m/s)", vel_buf, 9, ImGuiInputTextFlags_CharsDecimal);
  double max_vel = atof(vel_buf);
  
  bool create_disabled = false;
  
  static const char* err_text = "";
  if (!has_path) {
    err_text = "File save path required";
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
  
  if (ImGui::Button("Cancel")) {
    goto close;
  }
  
  ImGui::SameLine();

  if (create_disabled) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  if (ImGui::Button("Create")) {
    has_project = true;
    
    DriveController drivetrain = current_controller == 0 ? DriveController::RAMSETE : DriveController::HOLONOMIC;
    
    project = { path, drivetrain, atof(accel_buf), atof(decel_buf), atof(vel_buf) };
    
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

