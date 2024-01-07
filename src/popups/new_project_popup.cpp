#include <ThunderAuto/popups/new_project_popup.h>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/file_types.h>
#include <ThunderAuto/imgui_util.h>
#include <ThunderAuto/macro_util.h>

#define COLUMN_WIDTH 150.0f

void NewProjectPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false,
                          ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(400, 250));
  if (!ImGui::BeginPopupModal(m_name, nullptr, ImGuiWindowFlags_NoResize)) {
    return;
  }

  m_result = Result::NONE;

  static char deploy_path_buf[256] = "";
  bool has_deploy_path = false;

  static int current_field = 0;
  static int current_controller = 0;
  static float robot_length = 0.80645f;
  static float robot_width = 0.7112f;

  //
  // Save location.
  //
  {
    ImGuiScopedField field("Path", ICON_FA_FILE "  Path", COLUMN_WIDTH);

    ImGui::InputText("##Path", deploy_path_buf, 256, ImGuiInputTextFlags_None);
    std::string deploy_path_str = deploy_path_buf;

    ImGui::SameLine();

    if (ImGui::Button("Browse")) {
      deploy_path_str =
          m_platform_manager.save_file_dialog({THUNDERAUTO_FILE_FILTER});
      strncpy(deploy_path_buf, deploy_path_str.c_str(),
              deploy_path_str.length());
    }

    has_deploy_path = !deploy_path_str.empty();
  }

  //
  // Field.
  //
  {
    ImGuiScopedField field("Field", ICON_FA_SWIMMING_POOL "  Field",
                           COLUMN_WIDTH);

    const char* fields[] = {"2024 - Crescendo", "2023 - Charged Up", "2022 - Rapid React",
                            "Custom"};

    if (current_field != 3 && !m_field) {
      m_field = Field(static_cast<Field::BuiltinImage>(current_field));
    }

    const char* field_str;
    if (current_field == 3) {
      assert(m_field->type() == Field::ImageType::CUSTOM);
      field_str = m_field->custom_image_path().c_str();
    } else {
      field_str = fields[current_field];
    }

    if (ImGui::BeginCombo("##Field", field_str)) {
      for (int i = 0; i < 4; i++) {
        if (ImGui::Selectable(fields[i], current_field == i)) {
          current_field = i;
          m_field = std::nullopt;

          // If the user selects custom, open the new field popup.
          if (current_field == 3) {
            m_result = Result::NEW_FIELD;
          }
        }
      }
      ImGui::EndCombo();
    }
  }

  if (m_result == Result::NEW_FIELD) {
    *running = false;
    ImGui::EndPopup();
    return;
  }

  ImGui::Separator();

  //
  // Drive Controller Selection.
  //
  {
    ImGuiScopedField field("Controller Type", ICON_FA_CAR "  Controller Type",
                           COLUMN_WIDTH);

    const char* controllers[] = {"Holonomic (Swerve)", "Ramsete (Tank)"};

    if (ImGui::BeginCombo("##Controller Type",
                          controllers[current_controller])) {
      for (int i = 0; i < 2; i++) {
        if (ImGui::Selectable(controllers[i], current_controller == i,
                              ImGuiSelectableFlags_Disabled * (i == 1))) {
          current_controller = i;
        }
      }
      ImGui::EndCombo();
    }
  }

  ImGui::Separator();

  //
  // Robot Length.
  //
  {
    ImGuiScopedField field(
        "Robot Length", ICON_FA_RULER_VERTICAL "  Robot Length", COLUMN_WIDTH);

    ImGui::DragFloat("##Robot Length", &robot_length, 0.1f, 0.0f, 0.0f,
                     "%.2f m");

    if (robot_length < 0.0f) robot_length = 0.0f;
  }

  //
  // Robot Width.
  //
  {
    ImGuiScopedField field(
        "Robot Width", ICON_FA_RULER_HORIZONTAL "  Robot Width", COLUMN_WIDTH);

    ImGui::DragFloat("##Robot Width", &robot_width, 0.1f, 0.0f, 0.0f, "%.2f m");

    if (robot_width < 0.0f) robot_width = 0.0f;
  }

  //
  // Create & Cancel.
  //

  bool create_disabled = false;
  const char* error_text = nullptr;

  if (!has_deploy_path) {
    error_text = "Project path required";
    create_disabled = true;
  } else if (!robot_length) {
    error_text = "Robot length can not be 0 m";
    create_disabled = true;
  } else if (!robot_width) {
    error_text = "Robot width can not be 0 m";
    create_disabled = true;
  }

  // --- Cancel Button ---

  if (ImGui::Button("Cancel")) {
    m_result = Result::CANCEL;
  }

  ImGui::SameLine();

  // --- Create Button ---

  {
    ImGuiScopedDisabled disabled(create_disabled);

    if (ImGui::Button("Create")) {
      m_result = Result::CREATE;

      DriveController drivetrain = current_controller == 0
                                       ? DriveController::RAMSETE
                                       : DriveController::HOLONOMIC;

      std::filesystem::path project_path(deploy_path_buf);

      if (m_field.value().type() == Field::ImageType::CUSTOM) {
        project_path = project_path.parent_path();

        std::string image_path = m_field->custom_image_path();

        replace_macro(image_path, "PROJECT_DIR",
                      project_path.parent_path().string());

        std::cout << "image path " << image_path << std::endl;

        m_field = Field(image_path, m_field->image_rect());
      }

      m_project = ProjectSettings {.path = project_path,
                                   .field = m_field.value(),
                                   .drive_controller = drivetrain,
                                   .robot_length = robot_length,
                                   .robot_width = robot_width};
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
        error_text) {
      ImGui::SetTooltip("* %s", error_text);
    }
  }

  ImGui::EndPopup();

  if (m_result != Result::NONE) {
    *running = false;
  }
}
