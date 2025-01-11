#include <ThunderAuto/pages/properties_page.h>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.h>
#include <ThunderAuto/platform/platform.h>

#define COLUMN_WIDTH 135.0f

void PropertiesPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(name(), running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }

  ProjectState state = m_history.current_state();

  // Path Properties.
  if (ImGui::CollapsingHeader(ICON_FA_BEZIER_CURVE "  Path", nullptr,
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    present_path_properties(state);
  }

  // Point Properties.
  if (state.selected_point_index() != -1 &&
      ImGui::CollapsingHeader(ICON_FA_CIRCLE "  Point", nullptr,
                              ImGuiTreeNodeFlags_DefaultOpen)) {

    present_point_properties(state);
  }

  // Velocity Constraints.
  if (ImGui::CollapsingHeader("Velocity Constraints", nullptr,
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    present_velocity_properties(state);
  }

  // Editor Properties.
  if (ImGui::CollapsingHeader("Editor", nullptr,
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    present_editor_properties();
  }

  ImGui::End();
}

void PropertiesPage::present_point_properties(ProjectState& state) {
  std::size_t pt_index = state.selected_point_index();
  CurvePoint& pt = *state.selected_point();

  const Curve& curve = state.current_path();

  const bool is_first = pt_index == 0;
  const bool is_last = pt_index == curve.points().size() - 1;
  assert(!(is_first && is_last));

  bool changed = false;

  // Position
  changed |= edit_point_position(pt);

  const bool show_incoming = !is_first;
  const bool show_outgoing = !is_last;

  ImGui::Separator();

  // Heading
  changed |= edit_point_headings(pt, show_incoming, show_outgoing);

  // Heading weights.
  changed |= edit_point_heading_weights(pt, show_incoming, show_outgoing);

  ImGui::Separator();

  // Rotation.
  changed |= edit_point_rotation(pt);

  // Stop.
  if (!is_first && !is_last) {
    ImGui::Separator();
    changed |= edit_point_stop(pt);
  }

  if (changed) {
    state.update_linked_waypoints_from_selected();

    m_history.add_state(state);
    curve.output(m_cached_curve, preview_output_curve_settings);
  }

  const std::vector<std::string>& actions = state.actions();

  if (!actions.empty()) {
    ImGui::Separator();

    if (ImGui::TreeNode("Actions")) {
      for (std::size_t i = 0; i < actions.size(); ++i) {
        bool selected = pt.actions() & (1 << i);

        if (ImGui::Checkbox(("##action_" + std::to_string(i)).c_str(),
                            &selected)) {
          if (selected)
            pt.add_actions(1 << i);
          else
            pt.remove_actions(1 << i);

          m_history.add_state(state);
        }

        ImGui::SameLine();

        ImGui::Text("%s", actions.at(i).c_str());

        // Align to the right.
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 45.0f);

        ImGui::Text("1 << %d", (int)i);
      }

      ImGui::TreePop();
    }
  }
}

void PropertiesPage::present_path_properties(ProjectState& state) {
  Curve& curve = state.current_path();

  // Export to CSV.
  {
    ImGuiScopedField field("Export to CSV", COLUMN_WIDTH);

    if (ImGui::Button("Export")) {
      state.export_current_path_to_csv(*m_settings);
    }
  }

  ImGui::Separator();

  // Waypoint list

  std::vector<CurvePoint>& points = curve.points();

  ImGui::PushID("Waypoints");

  ImGui::Text("Waypoints");

  for (std::size_t i = 0; i < points.size(); ++i) {
    CurvePoint& pt = points.at(i);

    ImGui::PushID(i);
    ImGui::Columns(3, nullptr, false);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 70.0f - 20.0f);
    ImGui::SetColumnWidth(1, 30.0f);
    ImGui::SetColumnWidth(2, 40.0f);

    {
      ImGui::Indent(10.0f);

      if (ImGui::Selectable(std::to_string(i).c_str(),
                            int(i) == state.selected_point_index())) {
        state.selected_point_index() = i;
        m_history.add_state(state);
      }

      if (pt.link_index() != -1) {
        ImGui::SameLine();
        ImGui::Text(ICON_FA_ARROW_RIGHT "  %s",
                    state.waypoint_links().at(pt.link_index()).c_str());
      }

      ImGui::Unindent(10.0f);
    }

    ImGui::NextColumn();

    {
      bool locked = pt.editor_locked();
      if (ImGui::Button(locked ? ICON_FA_LOCK : ICON_FA_UNLOCK)) {
        pt.set_editor_locked(!locked);
        m_history.add_state(state);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(locked ? "Unlock" : "Lock");
      }
    }

    ImGui::NextColumn();

    {
      bool reset = false;

      if (ImGui::Button(ICON_FA_LINK)) {
        reset = true;

        ImGui::OpenPopup("Link Waypoint");
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false,
                                ImVec2(0.5f, 0.5f));
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Link to another point");
      }

      present_link_popup(state, i, reset);
    }

    ImGui::Columns(1);

    ImGui::PopID();
  }

  ImGui::PopID();
}

void PropertiesPage::present_velocity_properties(ProjectState& state) {
  Curve& curve = state.current_path();
  CurveSettings& settings = curve.settings();

  bool changed = false;

  // Linear Acceleration.
  {
    ImGuiScopedField field("Linear Accel", COLUMN_WIDTH);

    changed |= present_slider("##Linear Acceleration",
                              settings.max_linear_accel, 0.1f, "%.2f m/s²");

    settings.max_linear_accel =
        std::clamp(settings.max_linear_accel, 0.1f, 25.f);
  }

  // Linear Velocity.
  {
    ImGuiScopedField field("Linear Velocity", COLUMN_WIDTH);

    changed |= present_slider("##Linear Velocity", settings.max_linear_vel,
                              0.1f, "%.2f m/s");

    settings.max_linear_vel = std::clamp(settings.max_linear_vel, 0.1f, 25.f);
  }

  // Centripetal Acceleration.
  {
    ImGuiScopedField field("Centripetal Accel", COLUMN_WIDTH);

    changed |=
        present_slider("##Centripetal Acceleration",
                       settings.max_centripetal_accel, 0.25f, "%.2f m/s²");

    settings.max_centripetal_accel =
        std::clamp(settings.max_centripetal_accel, 0.1f, FLT_MAX);
  }

  if (changed) {
    m_history.add_state(state);
    state.current_path().output(m_cached_curve, preview_output_curve_settings);
  }
}

void PropertiesPage::present_editor_properties() {

  // Curve overlay.
  {
    ImGuiScopedField field("Curve Overlay", COLUMN_WIDTH);

    static PathEditorPage::CurveOverlay curve_overlay =
        PathEditorPage::CurveOverlay::VELOCITY;

    const char* curve_overlay_names[] = {
        "Velocity",
        "Curvature",
    };

    if (ImGui::Combo("##Curve Overlay", (int*)&curve_overlay,
                     curve_overlay_names, 2)) {
      m_path_editor_page.set_curve_overlay(curve_overlay);
    }
  }

  // Show tangents.
  {
    ImGuiScopedField field("Show Tangents", COLUMN_WIDTH);

    static bool show_tangents = true;
    if (ImGui::Checkbox("##Show Tangents", &show_tangents)) {
      m_path_editor_page.set_show_tangents(show_tangents);
    }
  }

  // Show rotation.
  {
    ImGuiScopedField field("Show Rotation", COLUMN_WIDTH);

    static bool show_rotation = true;
    if (ImGui::Checkbox("##Show Rotation", &show_rotation)) {
      m_path_editor_page.set_show_rotation(show_rotation);
    }
  }

  // Show curve tooltip.
  {
    ImGuiScopedField field("Show Tooltip", COLUMN_WIDTH);

    static bool show_tooltip = true;
    if (ImGui::Checkbox("##Show Tooltip", &show_tooltip)) {
      m_path_editor_page.set_show_tooltip(show_tooltip);
    }
  }
}

void PropertiesPage::present_link_popup(ProjectState& state,
                                        std::size_t point_index, bool reset) {
  CurvePoint& point = state.current_path().points().at(point_index);

  static int link_index = -1;
  static char link_buffer[256] = "";
  static bool new_link = false;

  std::vector<std::string>& links = state.waypoint_links();

  if (reset) {
    link_index = point.link_index();
    link_buffer[0] = '\0';
    new_link = false;
  }

  if (!ImGui::BeginPopupModal("Link Waypoint", nullptr,
                              ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoMove)) {
    return;
  }

  ImGui::SetWindowSize(ImVec2(300.f, -1.f));

  {
    ImGuiScopedField field("Link", 80.f);

    const char* combo_title = new_link ? "New Link"
                              : (link_index == -1)
                                  ? "None"
                                  : links.at(link_index).c_str();

    if (ImGui::BeginCombo("##link", combo_title)) {
      for (std::size_t j = 0; j < links.size(); ++j) {
        bool selected = (int(j) == link_index);
        if (ImGui::Selectable(links.at(j).c_str(), selected)) {
          link_index = j;
          new_link = false;
        }
      }

      ImGui::Separator();

      if (ImGui::Selectable("New Link", new_link)) {
        link_index = -1;
        new_link = true;
      }

      ImGui::Separator();

      if (ImGui::Selectable("None", link_index == -1)) {
        link_index = -1;
        new_link = false;
      }

      ImGui::EndCombo();
    }
  }

  if (new_link) {
    ImGuiScopedField field("Link Name", 80.f);

    ImGui::InputText("##input", link_buffer, 256);
  }

  {
    ImGuiScopedDisabled disabled(new_link && !strlen(link_buffer));

    if (ImGui::Button("Done")) {
      if (new_link) {
        link_index = links.size();

        std::string link_name = link_buffer;

        bool found_match = false;
        for (std::size_t i = 0; i < links.size(); ++i) {
          if (links.at(i) == link_name) {
            link_index = i;
            found_match = true;
            break;
          }
        }

        if (!found_match) {
          links.push_back(link_name);
        }
      }

      point.set_link_index(link_index);

      if (!new_link) {
        state.update_point_from_linked_waypoints(point_index);
        state.current_path().output(m_cached_curve,
                                    preview_output_curve_settings);
      }

      m_history.add_state(state);

      ImGui::CloseCurrentPopup();
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::CloseCurrentPopup();
  }

  ImGui::EndPopup();
}

bool PropertiesPage::edit_point_position(CurvePoint& pt) {
  ImVec2 position = pt.position();

  bool changed = false;
  {
    ImGuiScopedField field("X Position", COLUMN_WIDTH);
    changed |= present_slider("##X Position", position.x, 0.25f, "%.2f m");
  }
  {
    ImGuiScopedField field("Y Position", COLUMN_WIDTH);
    changed |= present_slider("##Y Position", position.y, 0.25f, "%.2f m");
  }

  pt.set_position(position);

  return changed;
}

bool PropertiesPage::edit_point_headings(CurvePoint& pt, bool incoming,
                                         bool outgoing) {
  auto show_heading_drag = [&](const char* id, bool which) -> bool {
    ImGuiScopedField field(id + 2, COLUMN_WIDTH);

    float heading = pt.heading(which).degrees();
    bool changed = present_slider(id, heading, 1.f, "%.2f°");

    pt.set_heading(Angle::degrees(heading), which);

    return changed;
  };

  bool result = false;

  if (incoming)
    result |= show_heading_drag("##Incoming Heading", CurvePoint::INCOMING);
  if (outgoing)
    result |= show_heading_drag("##Outgoing Heading", CurvePoint::OUTGOING);

  return result;
}

bool PropertiesPage::edit_point_heading_weights(CurvePoint& pt, bool incoming,
                                                bool outgoing) {
  auto show_weight_drag = [&](const char* id, bool which) -> bool {
    ImGuiScopedField field(id + 2, COLUMN_WIDTH);

    float weight = pt.heading_weight(which);
    bool changed = present_slider(id, weight, 0.25f);

    weight = std::clamp(weight, 0.1f, 25.f);
    pt.set_heading_weight(weight, which);

    return changed;
  };

  bool result = false;

  if (incoming)
    result |= show_weight_drag("##Incoming Weight", CurvePoint::INCOMING);
  if (outgoing)
    result |= show_weight_drag("##Outgoing Weight", CurvePoint::OUTGOING);

  return result;
}

bool PropertiesPage::edit_point_rotation(CurvePoint& pt) {
  ImGuiScopedField field("Rotation", COLUMN_WIDTH);

  float rotation = pt.rotation().degrees();
  bool changed = present_slider("##Rotation", rotation, 1.f, "%.2f°");

  pt.set_rotation(Angle::degrees(rotation));

  return changed;
}

bool PropertiesPage::edit_point_stop(CurvePoint& pt) {
  ImGuiScopedField field("Stop", COLUMN_WIDTH);

  bool stop = pt.stop();
  bool changed = ImGui::Checkbox("##Stop", &stop);

  pt.set_stop(stop);

  return changed;
}

bool PropertiesPage::present_slider(const char* id, float& value,
                                    const float speed, const char* format) {
  bool active = ImGui::DragFloat(id, &value, speed, 0.f, 0.f, format);

  if (ImGui::IsItemActivated()) {
    m_history.start_long_edit();
  }

  if (ImGui::IsItemDeactivated()) {
    if (ImGui::IsItemDeactivatedAfterEdit())
      m_history.finish_long_edit();
    else
      m_history.discard_long_edit();
  }

  return active;
}

