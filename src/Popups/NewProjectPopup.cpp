#include <ThunderAuto/Popups/NewProjectPopup.hpp>

#include <ThunderAuto/Platform/Platform.hpp>
#include <ThunderAuto/Error.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <IconsFontAwesome5.h>
#include <imgui.h>
#include <imgui_raii.h>
#include <fmt/format.h>

void NewProjectPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(NEW_PROJECT_POPUP_START_WIDTH), GET_UISIZE(NEW_PROJECT_POPUP_START_HEIGHT)));

  auto scopedWin = ImGui::Scoped::PopupModal(name(), running, ImGuiWindowFlags_NoResize);
  if (!scopedWin || !*running) {
    m_result = Result::CANCEL;
    return;
  }

  m_result = Result::NONE;

  // Save location
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_FILE "  Path").build();

    // Shrink the input text to make space for the browse button.
    {
      const ImGuiStyle& style = ImGui::GetStyle();
      float browseButtonWidth = ImGui::CalcTextSize("Browse").x + style.FramePadding.x * 2.f;
      ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - style.ItemSpacing.x - browseButtonWidth);
    }

    ImGui::InputText("##Path", m_projectPathBuf, sizeof(m_projectPathBuf));
    m_projectPath = m_projectPathBuf;

    ImGui::SameLine();

    if (ImGui::Button("Browse")) {
      std::filesystem::path selectedProjectPath = getPlatform().saveFileDialog({kThunderAutoFileFilter});
      if (!selectedProjectPath.empty()) {
        m_projectPath = selectedProjectPath;

        std::string pathStr = m_projectPath.string();
        strncpy(m_projectPathBuf, pathStr.c_str(), pathStr.size());
        m_projectPathBuf[pathStr.size()] = '\0';
      }
    }
  }

  // Field
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_SWIMMING_POOL "  Field").build();

    const char* fieldOptions[] = {"2025 - Reefscape", "2024 - Crescendo", "2023 - Charged Up",
                                  "2022 - Rapid React", "Custom"};
    const size_t numFieldOptions = sizeof(fieldOptions) / sizeof(fieldOptions[0]);
    const size_t customFieldOptionIndex = numFieldOptions - 1;

    // If the new field option was selected, but the user cancelled the new field popup, so there is no custom
    // field set.
    if (m_selectedFieldOptionIndex == customFieldOptionIndex) {
      if (!m_field || m_field->type() != ThunderAutoFieldImageType::CUSTOM) {
        m_selectedFieldOptionIndex = 0;  // Reset to default.
        m_field = std::nullopt;
      }
    }

    std::string fieldOptionPreviewStr = fieldOptions[m_selectedFieldOptionIndex];

    // Show the custom field image path in the combo box.
    if (m_selectedFieldOptionIndex == customFieldOptionIndex) {
      ThunderAutoAssert(m_field.has_value() && m_field->type() == ThunderAutoFieldImageType::CUSTOM);
      fieldOptionPreviewStr = m_field->customImagePath().string();
    }

    if (auto scopedCombo = ImGui::Scoped::Combo("##Field", fieldOptionPreviewStr.c_str())) {
      for (size_t i = 0; i < numFieldOptions; i++) {
        if (ImGui::Selectable(fieldOptions[i], m_selectedFieldOptionIndex == i)) {
          m_selectedFieldOptionIndex = i;
          m_field = std::nullopt;

          // Open the new field popup
          if (m_selectedFieldOptionIndex == customFieldOptionIndex) {
            m_result = Result::NEW_FIELD;
            *running = false;
            return;
          }
        }
      }
    }

    if (!m_field) {
      ThunderAutoAssert(m_selectedFieldOptionIndex != customFieldOptionIndex);
      auto builtinFieldImage = static_cast<ThunderAutoBuiltinFieldImage>(m_selectedFieldOptionIndex);
      m_field = ThunderAutoFieldImage(builtinFieldImage);
    }
  }

  if (m_result == Result::NEW_FIELD) {
    *running = false;
    return;
  }

  ImGui::Separator();

  // Drive Controller Selection
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_CAR "  Controller Type").build();

    const char* controllerOptions[] = {"Holonomic (Swerve)", "Ramsete (Tank)"};

    if (auto scopedCombo =
            ImGui::Scoped::Combo("##Controller Type", controllerOptions[m_selectedControllerOptionIndex])) {
      for (size_t i = 0; i < 2; i++) {
        if (ImGui::Selectable(controllerOptions[i], m_selectedControllerOptionIndex == i,
                              ImGuiSelectableFlags_Disabled * (i == 1U))) {
          m_selectedControllerOptionIndex = i;
        }
      }
    }
  }

  ImGui::Separator();

  // Robot Length
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_RULER_VERTICAL "  Robot Length").build();

    ImGui::DragFloat("##Robot Length", &m_robotLength, 0.1f, 0.0f, 0.0f, "%.2f m");
    m_robotLength = std::max(m_robotLength, 0.0f);
  }

  // Robot Width
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_RULER_HORIZONTAL "  Robot Width").build();

    ImGui::DragFloat("##Robot Width", &m_robotWidth, 0.1f, 0.0f, 0.0f, "%.2f m");
    m_robotWidth = std::max(m_robotWidth, 0.0f);
  }

  // Create & Cancel

  bool isCreateDisabled = false;
  std::string errorText;

  if (m_projectPath.empty()) {
    errorText = "Project path required";
    isCreateDisabled = true;

  } else if (m_projectPath.extension() != kThunderAutoFileExtension) {
    errorText = fmt::format("Project path must end with {}", kThunderAutoFileExtension);
    isCreateDisabled = true;

  } else if (!m_field.has_value()) {
    errorText = "Field image required";
    isCreateDisabled = true;

  } else if (m_selectedControllerOptionIndex >= 2) {
    errorText = "Invalid controller type selected";
    isCreateDisabled = true;

  } else if (m_robotLength < 1e-9) {
    errorText = "Robot length can not be 0 m";
    isCreateDisabled = true;
  } else if (m_robotWidth < 1e-9) {
    errorText = "Robot width can not be 0 m";
    isCreateDisabled = true;
  } else {
    std::filesystem::path projectDir = m_projectPath.parent_path();
    if (!projectDir.empty()) {
      if (!std::filesystem::exists(projectDir)) {
        errorText = fmt::format("Directory '{}' does not exist", projectDir.string());
        isCreateDisabled = true;
      } else if (!std::filesystem::is_directory(projectDir)) {
        errorText = fmt::format("Path '{}' is not a directory", projectDir.string());
        isCreateDisabled = true;
      }
    }
  }

  // --- Cancel Button ---

  if (ImGui::Button("Cancel")) {
    m_result = Result::CANCEL;
  }

  ImGui::SameLine();

  // --- Create Button ---

  bool showTooltip = false;
  {
    auto scopedDisabled = ImGui::Scoped::Disabled(isCreateDisabled);

    if (ImGui::Button("Create")) {
      m_result = Result::CREATE;

      m_project = ThunderAutoProjectSettings();

      m_projectPath = std::filesystem::absolute(m_projectPath);
      m_project.setProjectPath(m_projectPath);

      m_project.fieldImage = m_field.value();

      DriveControllerType drivetrain = static_cast<DriveControllerType>(m_selectedControllerOptionIndex);
      m_project.driveController = drivetrain;

      m_project.robotSize = {units::meter_t(m_robotLength), units::meter_t(m_robotWidth)};

      // Other values are set to defaults in the constructor.
    }

    showTooltip = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !errorText.empty();
  }
  if (showTooltip) {
    ImGui::SetTooltip("* %s", errorText.c_str());
  }

  if (m_result != Result::NONE) {
    *running = false;
  }
}
