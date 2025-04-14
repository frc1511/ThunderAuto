#include <ThunderAuto/pages/path_editor_page.hpp>

#include <ThunderAuto/imgui_util.hpp>
#include <ThunderAuto/macro_util.hpp>

#include <IconsFontAwesome5.h>
#include <stb_image.h>

static float HANDLE_THICKNESS(const ImRect& bb) {
  return bb.GetSize().x / 1000.0f;
}

static float LINE_THICKNESS(const ImRect& bb) {
  return bb.GetSize().x / 900.0f;
}

static float POINT_RADIUS(const ImRect& bb) {
  return max(min(4.5f, bb.GetSize().x / 250.0f), LINE_THICKNESS(bb) * 1.25f);
}

static const int32_t POINT_COLOR = IM_COL32(255, 255, 255, 255);
static const int32_t POINT_COLOR_SELECTED = IM_COL32(255, 242, 0, 255);

static const int32_t POINT_START_COLOR = IM_COL32(100, 255, 100, 255);
static const int32_t POINT_START_COLOR_SELECTED = IM_COL32(0, 255, 0, 255);

static const int32_t POINT_END_COLOR = IM_COL32(255, 100, 100, 255);
static const int32_t POINT_END_COLOR_SELECTED = IM_COL32(255, 0, 0, 255);

static const int32_t HANDLE_COLOR = IM_COL32(225, 225, 225, 255);
static const int32_t HANDLE_COLOR_SELECTED = IM_COL32(178, 169, 0, 255);

static const int32_t POINT_COLOR_PREVIEW = IM_COL32(128, 128, 128, 255);
static const int32_t POINT_COLOR_LOCKED = IM_COL32(192, 192, 192, 255);

static const int32_t ROTATION_DONE_COLOR = IM_COL32(255, 128, 0, 255);

static const ImVec2 to_screen_coordinate(const ImVec2& field_pt,
                                         const Field& field, const ImRect& bb) {
  ImVec2 pt = field_pt / field.size();

  pt *= field.image_rect().Max - field.image_rect().Min;
  pt += field.image_rect().Min;

  pt = ImVec2(pt.x, 1.f - pt.y);

  pt = bb.Min + pt * bb.GetSize();

  return pt;
}

static const ImVec2 to_field_coordinate(const ImVec2& screen_pt,
                                        const Field& field, const ImRect& bb) {
  ImVec2 pt = (screen_pt - bb.Min) / bb.GetSize();

  pt = ImVec2(pt.x, 1.f - pt.y);

  pt -= field.image_rect().Min;
  pt /= field.image_rect().Max - field.image_rect().Min;

  pt *= field.size();

  return pt;
}

static bool is_mouse_hovering_point(const ImVec2& pt, float tolerance_radius) {
  const ImVec2 click_tolerance(tolerance_radius, tolerance_radius);

  return ImGui::IsMouseHoveringRect(pt - click_tolerance, pt + click_tolerance);
}

#include <field_2022_png.h>
#include <field_2023_png.h>
#include <field_2024_png.h>
#include <field_2025_png.h>

void PathEditorPage::setup_field(const ProjectSettings& settings) {
  m_settings = &settings;

  if (settings.field.type() == Field::ImageType::CUSTOM) {
    std::string image_path(settings.field.custom_image_path());
    replace_macro(image_path, "PROJECT_DIR",
                  settings.path.parent_path().string());

    m_field_texture.init(image_path.c_str());
  } else {
    unsigned char* image_data_buf = nullptr;
    std::size_t image_data_size = 0;
    switch (settings.field.builtin_image()) {
      using enum Field::BuiltinImage;
    case FIELD_2022:
      image_data_buf = field_2022_png;
      image_data_size = field_2022_png_size;
      break;
    case FIELD_2023:
      image_data_buf = field_2023_png;
      image_data_size = field_2023_png_size;
      break;
    case FIELD_2024:
      image_data_buf = field_2024_png;
      image_data_size = field_2024_png_size;
      break;
    case FIELD_2025:
      image_data_buf = field_2025_png;
      image_data_size = field_2025_png_size;
      break;
    }

    m_field_texture.init(image_data_buf, image_data_size);
  }

  m_field_aspect_ratio = static_cast<float>(m_field_texture.width()) /
                         static_cast<float>(m_field_texture.height());

  if (!m_field_texture) {
    puts("Failed to load field image!!\n");
    return;
  }

  m_field_offset = ImVec2(0, 0);
  m_field_scale = 1.0f;

  m_history.current_state().current_path().output(
      m_cached_curve, preview_output_curve_settings);
}

void PathEditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(name(), running,
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse)) {
    ImGui::End();
    return;
  }

  m_is_focused = ImGui::IsWindowFocused();

  present_curve_editor();

  present_playback_slider();

  ImGui::End();
}

void PathEditorPage::present_curve_editor() {
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems) return;

  ImGui::SetScrollX(0);
  ImGui::SetScrollY(0);

  //
  // Setup canvas.
  //

  // Fit the canvas to the window.
  const ImVec2 window_size(ImGui::GetWindowSize());

  float dim_x = window_size.x;
  float dim_y = window_size.y;

  // Fit within the window size while maintaining aspect ratio.
  if ((dim_x / dim_y) > m_field_aspect_ratio)
    dim_x = dim_y * m_field_aspect_ratio;
  else
    dim_y = dim_x / m_field_aspect_ratio;

  ImVec2 canvas(dim_x, dim_y);

  //
  // Panning/Zooming.
  //
  if (ImGui::IsWindowHovered()) {
    pan_and_zoom();
  }

  // Apply pan/zoom offset.
  win->DC.CursorPos += m_field_offset;

  // Field bounding box, with zoom applied.
  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + canvas * m_field_scale);

  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0)) return;

  //
  // Field.
  //
  present_field(bb);

  //
  // Curve.
  //
  present_curve(bb);

  //
  // Robot Preview.
  //
  present_robot_preview(bb);

  //
  // Widgets.
  //
  ProjectState state = m_history.current_state();

  Curve& curve = state.current_path();

  for (std::size_t i = 0; i < curve.points().size(); ++i) {
    const int selected_index = state.selected_point_index();
    const bool selected = (int)i == selected_index;

    const bool first = (i == 0);
    const bool last = (i == curve.points().size() - 1);

    const CurvePoint& point = curve.points().at(i);

    if (m_show_rotation) present_point_rotation_widget(point, selected, bb);
    if (m_show_tangents)
      present_point_heading_widget(point, !first, !last, selected, bb);
    present_point_widget(point, selected, first, last, bb);
  }

  handle_input(state, bb);

  present_context_menus(state);
}

void PathEditorPage::pan_and_zoom() {
  const ImGuiIO& io = ImGui::GetIO();
  const ImGuiWindow* win = ImGui::GetCurrentWindow();

  //
  // Panning (Shift + Left-Click, or Middle-Click).
  //
  if ((io.KeyShift && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) ||
      ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
    m_field_offset += io.MouseDelta;
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
  }

  //
  // Zooming.
  //
  const float prev_field_scale = m_field_scale;

  m_field_scale += io.MouseWheel * 0.05f;
  m_field_scale = std::clamp(m_field_scale, 1.0f, 4.0f);

  // I CANNOT BEGIN TO EXPLAIN HOW HARD ZOOMING WAS TO FIGURE OUT.

  // Find the difference between mouse positions before and after the zoom,
  // offset by difference.

  const ImVec2 mouse_pos = io.MousePos - (win->DC.CursorPos + m_field_offset);

  const ImVec2 mouse_pos_diff =
      (mouse_pos / m_field_scale - mouse_pos / prev_field_scale) *
      m_field_scale;

  m_field_offset += mouse_pos_diff;
}

void PathEditorPage::present_context_menus(ProjectState& state) {
  const Curve& curve = state.current_path();

  if (ImGui::BeginPopup("EditorContextMenu")) {
    if (ImGui::BeginMenu(ICON_FA_PLUS "  Insert Point Here")) {
      if (ImGui::MenuItem("At Beginning")) {
        insert_point(state, 0, m_context_menu_position);
      }
      if (ImGui::MenuItem("At End")) {
        insert_point(state, curve.points().size(), m_context_menu_position);
      }
      {
        ImGuiScopedDisabled disable(state.selected_point_index() == -1);

        if (ImGui::MenuItem("Before Selected")) {
          insert_point(state, state.selected_point_index(),
                       m_context_menu_position);
        }
        if (ImGui::MenuItem("After Selected")) {
          insert_point(state, state.selected_point_index() + 1,
                       m_context_menu_position);
        }
      }

      ImGui::EndMenu();
    }

    ImGui::EndPopup();
  }

  if (ImGui::BeginPopup("EditorPointContextMenu")) {
    {
      ImGuiScopedDisabled disabled(state.current_path().points().size() <= 2);

      if (ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete Point")) {
        remove_selected_point(state);
      }
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginPopup("EditorCurveContextMenu")) {
    if (ImGui::MenuItem(ICON_FA_PLUS "  Insert Point Here")) {
      insert_point(state, m_context_menu_hovered_curve_point_index,
                   m_context_menu_position);
    }
    ImGui::EndPopup();
  }
}

void PathEditorPage::present_field(ImRect bb) {
  ImDrawList* draw_list(ImGui::GetWindowDrawList());

  // Background image.
  draw_list->AddImage(m_field_texture.id(), bb.Min, bb.Max);
}

void PathEditorPage::present_curve(ImRect bb) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  const Curve& curve = m_history.current_state().current_path();
  const std::vector<OutputCurvePoint>& points = m_cached_curve.points;

  const OutputCurvePoint* last_point = &points.front();

  for (std::size_t i = 1; i < points.size(); ++i) {
    const OutputCurvePoint* point = &points.at(i);

    const ImVec2 last_point_position =
        to_screen_coordinate(last_point->position, m_settings->field, bb);
    const ImVec2 point_position =
        to_screen_coordinate(point->position, m_settings->field, bb);

    last_point = point;

    float hue = 0.f;
    switch (m_curve_overlay) {
      using enum CurveOverlay;
    case VELOCITY:
      hue = 0.7f - point->velocity / curve.settings().max_linear_vel;
      break;
    case CURVATURE:
      hue = 0.6f - std::clamp(point->curvature, 0.f, 10.f) / 10.f;
      break;
    default:
      assert(false);
    }

    draw_list->AddLine(last_point_position, point_position,
                       ImColor::HSV(hue, 1.f, 1.f), LINE_THICKNESS(bb));
  }

  for (std::size_t i = 0; i < points.size(); ++i) {
    const OutputCurvePoint* point = &points.at(i);
    const ImVec2 point_position =
        to_screen_coordinate(point->position, m_settings->field, bb);
    if (point->flags & OUTPUT_CURVE_POINT_FLAG_ROTATION_DONE) {
      draw_list->AddCircleFilled(point_position, POINT_RADIUS(bb),
                                 ROTATION_DONE_COLOR, 4);
    }
  }
}

void PathEditorPage::present_point_widget(const CurvePoint& point,
                                          bool selected, bool first, bool last,
                                          ImRect bb) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImVec2 pt = to_screen_coordinate(point.position(), m_settings->field, bb);

  int32_t color = POINT_COLOR;
  int32_t color_selected = POINT_COLOR_SELECTED;

  if (first) {
    color = POINT_START_COLOR;
    color_selected = POINT_START_COLOR_SELECTED;
  } else if (last) {
    color = POINT_END_COLOR;
    color_selected = POINT_END_COLOR_SELECTED;
  }

  int32_t final_color = selected ? color_selected : color;

  if (point.editor_locked()) {
    final_color = POINT_COLOR_LOCKED;
  }

  draw_list->AddCircleFilled(pt, POINT_RADIUS(bb), final_color);
}

void PathEditorPage::present_point_rotation_widget(const CurvePoint& point,
                                                   bool selected, ImRect bb) {
  ImDrawList* draw_list(ImGui::GetWindowDrawList());

  int32_t color = POINT_COLOR;

  if (point.editor_locked()) {
    color = POINT_COLOR_LOCKED;
  } else if (selected) {
    color = POINT_COLOR_SELECTED;
  }

  ImVec2 pt = to_screen_coordinate(
      point.rotation_control_point(m_settings->robot_length), m_settings->field,
      bb);
  draw_list->AddCircleFilled(pt, POINT_RADIUS(bb), color);

  std::array<ImVec2, 4> corners =
      point.robot_corners(m_settings->robot_length, m_settings->robot_width);
  for (ImVec2& corner : corners) {
    corner = to_screen_coordinate(corner, m_settings->field, bb);
  }

  draw_list->AddQuad(corners[0], corners[1], corners[2], corners[3], color,
                     HANDLE_THICKNESS(bb));
}

void PathEditorPage::present_point_heading_widget(const CurvePoint& point,
                                                  bool incoming, bool outgoing,
                                                  bool selected, ImRect bb) {
  if (point.editor_locked()) return;

  ImDrawList* draw_list(ImGui::GetWindowDrawList());

  const ImVec2 pt =
      to_screen_coordinate(point.position(), m_settings->field, bb);

  const int32_t handle_color = selected ? HANDLE_COLOR_SELECTED : HANDLE_COLOR;
  const int32_t point_color = selected ? POINT_COLOR_SELECTED : POINT_COLOR;

  if (incoming) {
    const ImVec2 control_pt =
        to_screen_coordinate(point.heading_control_point(CurvePoint::INCOMING),
                             m_settings->field, bb);

    draw_list->AddLine(pt, control_pt, handle_color, HANDLE_THICKNESS(bb));

    draw_list->AddCircleFilled(control_pt, POINT_RADIUS(bb), point_color);
  }
  if (outgoing) {
    const ImVec2 control_pt =
        to_screen_coordinate(point.heading_control_point(CurvePoint::OUTGOING),
                             m_settings->field, bb);

    draw_list->AddLine(pt, control_pt, handle_color, HANDLE_THICKNESS(bb));

    draw_list->AddCircleFilled(control_pt, POINT_RADIUS(bb), point_color);
  }
}

void PathEditorPage::present_robot_preview(ImRect bb) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  // Find closest time.

  std::vector<OutputCurvePoint>& points = m_cached_curve.points;

  std::vector<OutputCurvePoint>::iterator lower_it =
      std::lower_bound(points.begin(), points.end(), m_playback_time,
                       [](const OutputCurvePoint& point, float time) {
                         return point.time < time;
                       });
  std::vector<OutputCurvePoint>::iterator upper_it;

  float playback_time = m_playback_time;

  if (lower_it == points.end()) {
    upper_it = std::prev(lower_it);
    lower_it = std::prev(upper_it);

  } else if (lower_it == points.begin()) {
    upper_it = std::next(lower_it);
    if (upper_it == points.end()) {
      upper_it = lower_it;
    }
    playback_time = lower_it->time;

  } else {
    upper_it = lower_it;
    lower_it = std::prev(lower_it);
  }

  assert(lower_it != points.end());
  assert(upper_it != points.end());

  const OutputCurvePoint& lower_point = *lower_it;
  const OutputCurvePoint& upper_point = *upper_it;

  const float dt = upper_point.time - lower_point.time;
  const float t = dt > 0.01f ? (playback_time - lower_point.time) / dt : 0.f;

  const ImVec2 position =
      ImVec2(std::lerp(lower_point.position.x, upper_point.position.x, t),
             std::lerp(lower_point.position.y, upper_point.position.y, t));

  Angle rotation;
  {
    float rotation_delta_rad =
        upper_point.rotation.radians() - lower_point.rotation.radians();

    const float pi = std::numbers::pi_v<float>;
    if (rotation_delta_rad > pi) {
      rotation_delta_rad -= 2.f * pi;
    } else if (rotation_delta_rad < -pi) {
      rotation_delta_rad += 2.f * pi;
    }

    float rotation_rad =
        lower_point.rotation.radians() + rotation_delta_rad * t;

    rotation = Angle::radians(rotation_rad, Angle::Bounds::NEG_180_TO_POS_180);
  }

  ImVec2 rotation_pt = to_screen_coordinate(
      pt_extend_at_angle(position, rotation, m_settings->robot_length / 2.f),
      m_settings->field, bb);

  draw_list->AddCircleFilled(rotation_pt, POINT_RADIUS(bb),
                             POINT_COLOR_PREVIEW);

  std::array<ImVec2, 4> corners = robot_corners(
      position, rotation, m_settings->robot_length, m_settings->robot_width);
  for (ImVec2& corner : corners) {
    corner = to_screen_coordinate(corner, m_settings->field, bb);
  }

  draw_list->AddQuad(corners[0], corners[1], corners[2], corners[3],
                     POINT_COLOR_PREVIEW, HANDLE_THICKNESS(bb));
}

void PathEditorPage::present_playback_slider() {
  // Bottom of the window.
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 30);

  // Play button.
  if (ImGui::Button(m_is_playing ? ICON_FA_PAUSE : ICON_FA_PLAY)) {
    m_is_playing = !m_is_playing;
  }
  ImGui::SameLine();

  // Reset button.
  if (ImGui::Button(ICON_FA_REDO)) {
    m_playback_time = 0.f;
  }
  ImGui::SameLine();

  // Slider.
  assert(m_cached_curve.points.size() > 0);
  float total_time = m_cached_curve.points.back().time;

  if (m_is_playing) {
    m_playback_time += ImGui::GetIO().DeltaTime;
    m_playback_time = std::fmod(m_playback_time, total_time);
  }

  ImGui::PushItemWidth(ImGui::GetWindowWidth() - 75);

  {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f / %.2f s", m_playback_time,
             total_time);

    ImGui::SliderFloat("##Playback", &m_playback_time, 0.0f, total_time,
                       buffer);
  }

  ImGui::PopItemWidth();
}

void PathEditorPage::toggle_playback() { m_is_playing = !m_is_playing; }

void PathEditorPage::handle_input(ProjectState& state, ImRect bb) {
  m_show_context_menu = false;

  handle_point_input(state, bb);

  if (ImGui::IsWindowHovered()) {
    if (m_drag_point == PointType::NONE) {
      handle_curve_input(state, bb);
    }

    if (!m_show_context_menu && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      ImVec2 mouse_pos = ImGui::GetIO().MousePos;
      ImGui::OpenPopup("EditorContextMenu");
      m_show_context_menu = true;
    }
  }
}

void PathEditorPage::handle_point_input(ProjectState& state, ImRect bb) {
  const ImGuiIO& io = ImGui::GetIO();

  Curve& curve = state.current_path();

  int hovered_point_index = -1;
  PointType hovered_point_type = PointType::NONE;

  for (std::size_t i = 0; i < curve.points().size(); ++i) {
    CurvePoint& point = curve.points().at(i);

    if (point.editor_locked()) continue;

    const ImVec2 position_pt = point.position();
    const auto [heading_in_pt, heading_out_pt] = point.heading_control_points();
    const ImVec2 rotation_pt =
        point.rotation_control_point(m_settings->robot_length);

    const auto check_pt = [&](const ImVec2& pt, PointType type) {
      if (is_mouse_hovering_point(
              to_screen_coordinate(pt, m_settings->field, bb),
              POINT_RADIUS(bb) * 1.25f)) {
        hovered_point_type = type;
      }
    };

    check_pt(position_pt, PointType::POSITION);
    if (m_show_tangents) {
      check_pt(heading_in_pt, PointType::HEADING_IN);
      check_pt(heading_out_pt, PointType::HEADING_OUT);
    }
    if (m_show_rotation) {
      check_pt(rotation_pt, PointType::ROTATION);
    }

    if (hovered_point_type != PointType::NONE) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
      hovered_point_index = i;
      break;
    }
  }

  if (!ImGui::IsWindowHovered()) {
    if (m_drag_point != PointType::NONE) {
      // End drag.
      state.update_linked_waypoints_from_selected();
      m_history.add_state(state);

      m_history.finish_long_edit();

      m_is_playing = m_was_playing;

      curve.output(m_cached_curve, preview_output_curve_settings);
    }

    m_drag_point = PointType::NONE;
    m_clicked_point = PointType::NONE;

    return;
  }

  bool right_click = ImGui::IsMouseClicked(ImGuiMouseButton_Right);

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
      (right_click && (hovered_point_type == PointType::POSITION))) {

    int prev_selected_point_index = state.selected_point_index();

    state.selected_point_index() = hovered_point_index;
    m_clicked_point = hovered_point_type;

    if (prev_selected_point_index != hovered_point_index) {
      m_history.add_state(state, false);
    }

    if (right_click && !m_show_context_menu && (hovered_point_index != -1)) {
      ImGui::OpenPopup("EditorPointContextMenu");
      m_show_context_menu = true;
    }
  }

  if (m_clicked_point != PointType::NONE || m_drag_point != PointType::NONE) {
    // Dragging.
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

      state.update_linked_waypoints_from_selected();
      curve.output(m_cached_curve, preview_output_curve_settings);

      if (m_clicked_point != PointType::NONE) {
        // Begin drag.
        m_history.start_long_edit();

        m_was_playing = m_is_playing;
        m_is_playing = false;
        m_playback_time = 0.f;

        m_drag_point = m_clicked_point;
      }

      const ImVec2 new_position =
          to_field_coordinate(io.MousePos, m_settings->field, bb);

      CurvePoint& point = curve.points().at(state.selected_point_index());

      switch (m_drag_point) {
        using enum PointType;
      case POSITION:
        point.set_position(new_position);
        break;
      case HEADING_IN:
        point.set_heading_control_point(new_position, CurvePoint::INCOMING);
        break;
      case HEADING_OUT:
        point.set_heading_control_point(new_position, CurvePoint::OUTGOING);
        break;
      case ROTATION:
        point.set_rotation_control_point(new_position);
        break;
      default:
        assert(false);
      }

      m_history.add_state(state);

      m_clicked_point = PointType::NONE;
    }

    // Release.
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      if (m_drag_point != PointType::NONE) {
        // End drag.
        state.update_linked_waypoints_from_selected();
        m_history.add_state(state);

        m_history.finish_long_edit();

        m_is_playing = m_was_playing;

        curve.output(m_cached_curve, preview_output_curve_settings);
      }

      m_drag_point = PointType::NONE;
      m_clicked_point = PointType::NONE;
    }
  }
}

void PathEditorPage::handle_curve_input(ProjectState& state, ImRect bb) {
  const ImGuiIO& io = ImGui::GetIO();

  Curve& curve = state.current_path();

  const std::vector<OutputCurvePoint>& points = m_cached_curve.points;

  bool hovered = false;
  const OutputCurvePoint* hovered_point = nullptr;

  for (std::size_t i = 0; i < points.size(); ++i) {
    const OutputCurvePoint* point = &points.at(i);

    const ImVec2 point_position =
        to_screen_coordinate(point->position, m_settings->field, bb);

    if (!hovered &&
        is_mouse_hovering_point(point_position, LINE_THICKNESS(bb) * 2.f)) {
      hovered = true;
      hovered_point = point;

      if (m_show_tooltip) {
        ImGui::BeginTooltip();
        ImGui::Text("Time: %.2f s", point->time);
        ImGui::Text("Linear Velocity: %.2f m/s", point->velocity);
        ImGui::Text("Centripetal Accel: %.2f m/s²", point->centripetal_accel);
        ImGui::Text("Menger Curvature: %.2f", point->curvature);
        ImGui::Text("Radius of Curvature: %.2f m",
                    std::pow(point->curvature, -1));
        ImGui::Text("Rotation: %.2f deg", point->rotation.degrees());
        ImGui::Text("Angular Velocity: %.2f deg/s",
                    point->angular_velocity * 180 / std::numbers::pi_v<float>);

        ImGui::EndTooltip();
      }
    }
  }

  std::size_t hovered_point_index = curve.points().size();
  const ImVec2 mouse_pos =
      to_field_coordinate(io.MousePos, m_settings->field, bb);

  if (hovered) {
    assert(hovered_point);
    hovered_point_index = hovered_point->segment_index + 1;

  } else {
    const float distance_from_front =
        pt_distance(mouse_pos, curve.points().front().position());
    const float distance_from_back =
        pt_distance(mouse_pos, curve.points().back().position());

    if (distance_from_front < distance_from_back) {
      hovered_point_index = 0;
    }
  }

  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
    insert_point(state, hovered_point_index, mouse_pos);
  }
  if (!m_show_context_menu && hovered &&
      ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
    ImVec2 mouse_pos = ImGui::GetIO().MousePos;
    m_context_menu_position =
        to_field_coordinate(mouse_pos, m_settings->field, bb);
    m_context_menu_hovered_curve_point_index = hovered_point_index;
    ImGui::OpenPopup("EditorCurveContextMenu");
    m_show_context_menu = true;
  }

  if (state.selected_point_index() != -1 && curve.points().size() > 2 &&
      (ImGui::IsKeyPressed(ImGuiKey_Delete) ||
       ImGui::IsKeyPressed(ImGuiKey_Backspace))) {
    remove_selected_point(state);
  }
}

void PathEditorPage::insert_point(ProjectState& state, std::size_t index,
                                  ImVec2 position) {
  Curve& curve = state.current_path();

  curve.insert_point(index, position);
  state.selected_point_index() = index;
  m_history.add_state(state);

  curve.output(m_cached_curve, preview_output_curve_settings);
}

void PathEditorPage::select_next_point(ProjectState& state) {
  Curve& curve = state.current_path();

  if (state.selected_point_index() == -1 ||
      state.selected_point_index() == int(curve.points().size() - 1))
    return;

  state.selected_point_index() += 1;
  m_history.add_state(state);
}

void PathEditorPage::select_previous_point(ProjectState& state) {
  if (state.selected_point_index() <= 0) return;

  state.selected_point_index() -= 1;
  m_history.add_state(state);
}

void PathEditorPage::remove_selected_point(ProjectState& state) {
  Curve& curve = state.current_path();

  if (curve.points().size() <= 2) return;

  curve.remove_point(state.selected_point_index());
  if (state.selected_point_index()) {
    state.selected_point_index() -= 1;
  }
  m_history.add_state(state);

  curve.output(m_cached_curve, preview_output_curve_settings);
}

