#include <ThunderAuto/Pages/ProjectSettingsPage.hpp>

#include <ThunderAuto/FontLibrary.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/Error.hpp>
#include <IconsFontAwesome5.h>
#include <imgui_raii.h>

void ProjectSettingsPage::present(bool* running) {
  ImGui::ShowDemoWindow();

  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(PROJECT_SETTINGS_PAGE_START_WIDTH), GET_UISIZE(PROJECT_SETTINGS_PAGE_START_HEIGHT)),
      ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || !*running)
    return;

  // Side bar

  {
    auto scopedItemSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

    const float kSideBarWidthPercent = 0.3f;
    auto scopedChild = ImGui::Scoped::ChildWindow(
        "SubPageSelectChild", ImVec2(ImGui::GetContentRegionAvail().x * kSideBarWidthPercent, 0.f),
        ImGuiChildFlags_Borders);

    if (ImGui::Selectable("Robot Settings", m_subPage == SettingsSubPage::ROBOT_SETTINGS)) {
      m_subPage = SettingsSubPage::ROBOT_SETTINGS;
    }

    if (ImGui::Selectable("CSV Export Settings", m_subPage == SettingsSubPage::CSV_EXPORT_SETTINGS)) {
      m_subPage = SettingsSubPage::CSV_EXPORT_SETTINGS;
    }
    if (ImGui::Selectable("Trajectory Editor Settings",
                          m_subPage == SettingsSubPage::TRAJECTORY_EDITOR_SETTINGS)) {
      m_subPage = SettingsSubPage::TRAJECTORY_EDITOR_SETTINGS;
    }
  }

  ImGui::SameLine();

  {
    auto scopedChild = ImGui::Scoped::ChildWindow("SubPageChild");

    switch (m_subPage) {
      case SettingsSubPage::ROBOT_SETTINGS:
        presentRobotSettings();
        break;
      case SettingsSubPage::CSV_EXPORT_SETTINGS:
        presentCSVExportSettings();
        break;
      case SettingsSubPage::TRAJECTORY_EDITOR_SETTINGS:
        presentTrajectoryEditorSettings();
        break;
      default:
        ThunderAutoUnreachable("Invalid project settings sub page");
    }
  }
}

void ProjectSettingsPage::presentRobotSettings() {
  ThunderAutoProjectSettings& settings = m_documentManager.settings();

  // Title
  {
    auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
    ImGui::Text("Robot Settings");

    ImGui::Spacing();
  }

  float robotLength = static_cast<float>(settings.robotSize.length.value());
  float robotWidth = static_cast<float>(settings.robotSize.width.value());

  const float lastRobotLength = robotLength, lastRobotWidth = robotWidth;

  // Robot Length.
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_RULER_VERTICAL "      Robot Length").build();

    bool active = ImGui::DragFloat("##Robot Length", &robotLength, 0.1f, 0.0f, 5.0f, "%.2f m",
                                   ImGuiSliderFlags_AlwaysClamp);
    if (active && robotLength != lastRobotLength) {
      settings.robotSize.length = units::meter_t(static_cast<double>(robotLength));
      m_documentManager.history().markUnsaved();
    }
  }

  // Robot Width.
  {
    auto scopedField = ImGui::ScopedField::Builder(ICON_FA_RULER_HORIZONTAL "  Robot Width").build();

    bool active = ImGui::DragFloat("##Robot Width", &robotWidth, 0.1f, 0.0f, 5.0f, "%.2f m",
                                   ImGuiSliderFlags_AlwaysClamp);
    if (active && robotWidth != lastRobotWidth) {
      settings.robotSize.width = units::meter_t(static_cast<double>(robotWidth));
      m_documentManager.history().markUnsaved();
    }
  }
}
void ProjectSettingsPage::presentCSVExportSettings() {
  ThunderAutoProjectSettings& settings = m_documentManager.settings();

  // Title
  {
    auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
    ImGui::Text("CSV Export Settings");

    ImGui::Spacing();
  }

  ThunderAutoCSVExportProperties& exportProperties = settings.csvExportProps;
  bool changed = false;

  if (ImGui::Button("Default")) {
    exportProperties = ThunderAutoCSVExportProperties::Default();
  }
  ImGui::SameLine();
  if (ImGui::Button("Legacy Default (Pre-2026)")) {
    exportProperties = ThunderAutoCSVExportProperties::LegacyDefault();
  }
  ImGui::SameLine();
  if (ImGui::Button("Full")) {
    exportProperties = ThunderAutoCSVExportProperties::Full();
  }

  const float fieldLeftColumnWidth = GET_UISIZE(FIELD_NORMAL_LEFT_COLUMN_WIDTH) * 1.25f;

  ImGui::SeparatorText("CSV Format");

  {
    auto scopedField = ImGui::ScopedField::Builder("Include Header")
                           .leftColumnWidth(fieldLeftColumnWidth)
                           .tooltip("Write a header to the first line of the CSV with column names")
                           .build();
    changed |= ImGui::Checkbox("##Include Header", &exportProperties.includeHeader);
  }

  ImGui::SeparatorText("CSV Values");

  {
    auto scopedField = ImGui::ScopedField::Builder("Time").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##time", &exportProperties.time);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Position").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##position", &exportProperties.position);
  }
  {
    auto scopedField =
        ImGui::ScopedField::Builder("Linear Velocity").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##linearVelocity", &exportProperties.linearVelocity);
  }
  {
    auto scopedField =
        ImGui::ScopedField::Builder("Component Velocities").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##componentVelocities", &exportProperties.componentVelocities);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Heading").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##heading", &exportProperties.heading);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Rotation").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##rotation", &exportProperties.rotation);
  }
  {
    auto scopedField =
        ImGui::ScopedField::Builder("Angular Velocity").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##angularVelocity", &exportProperties.angularVelocity);
  }
  {
    auto scopedField =
        ImGui::ScopedField::Builder("Actions Bit  Field").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##actionsBitField", &exportProperties.actionsBitField);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Distance").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##distance", &exportProperties.distance);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Curvature").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##curvature", &exportProperties.curvature);
  }
  {
    auto scopedField =
        ImGui::ScopedField::Builder("Centripetal Acceleration").leftColumnWidth(fieldLeftColumnWidth).build();
    changed |= ImGui::Checkbox("##centripetalAcceleration", &exportProperties.centripetalAcceleration);
  }

  if (changed) {
    m_documentManager.history().markUnsaved();
  }
}
void ProjectSettingsPage::presentTrajectoryEditorSettings() {
  // Title
  {
    auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
    ImGui::Text("Trajectory Editor Settings");

    ImGui::Spacing();
  }

  EditorPage::TrajectoryEditorOptions& options = m_editorPage.trajectoryEditorOptions;

  {
    auto scopedField = ImGui::ScopedField::Builder("Show Tangents").build();
    ImGui::Checkbox("##Show Tangents", &options.showTangents);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Show Rotations").build();
    ImGui::Checkbox("##Show Rotations", &options.showRotations);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Show Actions").build();
    ImGui::Checkbox("##Show Actions", &options.showActions);
  }
  {
    auto scopedField = ImGui::ScopedField::Builder("Show Tooltip").build();
    ImGui::Checkbox("##Show Tooltip", &options.showTooltip);
  }
}
