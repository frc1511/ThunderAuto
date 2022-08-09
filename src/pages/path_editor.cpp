#include <pages/path_editor.h>
#include <pages/path_manager.h>
#include <project.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <IconsFontAwesome5.h>

#define CURVE_RESOLUTION_FACTOR 64.0f
#define CURVE_THICKNESS 2.0f
#define TANGENT_THICKNESS 1.0f
#define POINT_RADIUS 5.0f
#define POINT_BORDER_THICKNESS 2.0f
#define INTEGRAL_PRECISION 0.001f

#define CTRL_POINT_IMPACT 1.0f

// The threshold for the curvature of a curve to slow down the robot.
#define CURVATURE_THRESHOLD 15.0f
// The maximum velocity of the robot when the curvature is above the threshold.
#define SLOW_CRUISE_VELOCITY 0.5f // m/s

std::optional<ImVec2> PathEditorPage::CurvePoint::get_tangent_pt(bool first) const {
  if ((!first && begin) || (first && end)) {
    return std::nullopt;
  }

  float dx = std::cos((first ? h0 : h1) + (M_PI * !first)) * (first ? w0 : w1),
        dy = std::sin((first ? h0 : h1) + (M_PI * !first)) * (first ? w0 : w1);

  return ImVec2(px + dx, py + dy);
}

void PathEditorPage::CurvePoint::set_tangent_pt(bool first, float x, float y) {
  float dx(x - px),
        dy(y - py);

  float &_h0 = (first ? h0 : h1),
        &_h1 = (first ? h1 : h0),
        &w = (first ? w0 : w1);

  _h0 = std::atan2(dy, dx) + (M_PI * !first);

  if (!stop) {
    _h1 = _h0;
  }

  w = std::hypotf(dx, dy);
}

ImVec2 PathEditorPage::CurvePoint::get_rot_pt(bool reverse) const {
  float v(reverse * 2 - 1);

  ImVec2 pt(px, py);

  pt.x += std::cos(rotation + M_PI * v) * (project->settings.robot_length / 2) * v;
  pt.y += std::sin(rotation + M_PI * v) * (project->settings.robot_length / 2) * v;

  return pt;
}

ImVec2 PathEditorPage::CurvePoint::get_rot_corner_pt(int index) const {
  bool reverse = false;
  if (index > 1) {
    reverse = true;
    index -= 2;
  }

  float v((2 * index) - 1);

  float dx(std::cos(rotation + M_PI_2 * v) * (project->settings.robot_width / 2)),
        dy(std::sin(rotation + M_PI_2 * v) * (project->settings.robot_width / 2));

  ImVec2 pt(get_rot_pt(reverse));

  return ImVec2(pt.x + dx, pt.y + dy);
}

void PathEditorPage::CurvePoint::set_rot_pt(float x, float y) {
  float dx(x - px),
        dy(y - py);

  rotation = std::atan2(dy, dx);
}

void PathEditorPage::CurvePoint::translate(float dx, float dy) {
  px += dx;
  py += dy;
}

PathEditorPage::PathEditorPage() { }

PathEditorPage::~PathEditorPage() { }

ImVec2 PathEditorPage::to_field_coord(ImVec2 pt, bool apply_offset) const {
  if (apply_offset) {
    pt /= field_scale;
  }

  pt = ImVec2((pt.x - bb.Min.x) / (bb.Max.x - bb.Min.x), 1 - (pt.y - bb.Min.y) / (bb.Max.y - bb.Min.y));
  pt.x *= FIELD_X;
  pt.y *= FIELD_Y;

  pt = un_adjust_field_coord(pt);

  if (apply_offset) {
    pt -= field_offset;
  }

  return pt;
}

ImVec2 PathEditorPage::to_draw_coord(ImVec2 pt, bool apply_offset) const {
  if (apply_offset) {
    pt += field_offset;
  }

  pt = adjust_field_coord(pt);

  pt.x /= FIELD_X;
  pt.y /= FIELD_Y;
  pt = ImVec2(pt.x, 1 - pt.y) * (bb.Max - bb.Min) + bb.Min;

  if (apply_offset) {
    pt *= field_scale;
  }

  return pt;
}

ImVec2 PathEditorPage::adjust_field_coord(ImVec2 pt) const {
  const Field& field(project->settings.field);

  pt.x /= FIELD_X;
  pt.y /= FIELD_Y;

  pt.x *= (field.max.x - field.min.x);
  pt.y *= (field.max.y - field.min.y);

  pt.x += field.min.x;
  pt.y += field.min.y;

  pt.x *= FIELD_X;
  pt.y *= FIELD_Y;

  return pt;
}

ImVec2 PathEditorPage::un_adjust_field_coord(ImVec2 pt) const {
  const Field& field(project->settings.field);

  pt.x /= FIELD_X;
  pt.y /= FIELD_Y;

  pt.x -= field.min.x;
  pt.y -= field.min.y;

  pt.x /= (field.max.x - field.min.x);
  pt.y /= (field.max.y - field.min.y);

  pt.x *= FIELD_X;
  pt.y *= FIELD_Y;

  return pt;
}

std::optional<PathEditorPage::CurvePointTable::iterator> PathEditorPage::get_selected_point() {
  if (selected_pt == PathManagerPage::get()->get_selected_path().cend()) {
    return std::nullopt;
  }
  return selected_pt;
}

void PathEditorPage::reset_selected_point() {
  selected_pt = PathManagerPage::get()->get_selected_path().end();
}

void PathEditorPage::delete_point() {
    if (selected_pt != PathManagerPage::get()->get_selected_path().end() && PathManagerPage::get()->get_selected_path().size() > 1) {
      bool is_end = selected_pt == PathManagerPage::get()->get_selected_path().end() - 1,
           is_begin = selected_pt == PathManagerPage::get()->get_selected_path().begin();

      PathManagerPage::get()->get_selected_path().erase(selected_pt);

      if (is_begin) {
        selected_pt = PathManagerPage::get()->get_selected_path().begin();
        selected_pt->begin = true;
      }
      else {
        selected_pt = PathManagerPage::get()->get_selected_path().end() - is_end;
        selected_pt->end = true;
      }

      selected_pt->stop = false;
      selected_pt->h1 = selected_pt->h0;

      updated = true;
    }
}

void PathEditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Editor", running,
                ImGuiWindowFlags_NoCollapse
              | ImGuiWindowFlags_NoScrollbar
              | ImGuiWindowFlags_NoScrollWithMouse
              | ImGuiWindowFlags_UnsavedDocument * ProjectManager::get()->is_unsaved())) {
    ImGui::End();
    return;
  }

  focused = ImGui::IsWindowFocused();

  if (ProjectManager::get()->has_project()) {
    present_curve_editor();
  }
  
  ImGui::End();
}

#include <field_2022_png.h>

void PathEditorPage::set_project(Project* _project) {
  project = _project;
  selected_pt = PathManagerPage::get()->get_selected_path().end();

  auto replace_macro = [](std::string& str, std::string macro, std::string value) {
    std::size_t pos;
    while (pos = str.find("${" + macro + "}"), pos != std::string::npos) {
      str.replace(pos, macro.length() + 3, value);
    }
  };

  int width, height, nr_channels;
  unsigned char* img_data;
  if (project->settings.field.img_type == Field::ImageType::CUSTOM) {
    std::string img_path(std::get<std::string>(project->settings.field.img));
    replace_macro(img_path, "PROJECT_DIR", project->settings.path.parent_path().string());
    img_data = stbi_load(img_path.c_str(), &width, &height, &nr_channels, 0);
  }
  else {
    const unsigned char* img_data_buf = nullptr;
    std::size_t img_data_size = 0;
    switch (std::get<Field::BuiltinImage>(project->settings.field.img)) {
      case Field::BuiltinImage::FIELD_2022:
        img_data_buf = field_2022_png;
        img_data_size = field_2022_png_size;
        break;
    }

    img_data = stbi_load_from_memory(img_data_buf, img_data_size, &width, &height, &nr_channels, 0);
  }

  if (img_data) {
    int tex_channels(nr_channels == 3 ? GL_RGB : GL_RGBA);

    glGenTextures(1, &field_tex);
    glBindTexture(GL_TEXTURE_2D, field_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, tex_channels, width, height, 0, tex_channels, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(img_data);
    field_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
  }
  else {
    std::cout << "Failed to load texture" << std::endl;
  }

  // Reset stuff.
  field_offset = ImVec2(0.0f, 0.0f);
  field_scale = 1.0f;

  updated = true;
}

void PathEditorPage::export_path() {
  // Update the values.
  cache_values();

  std::filesystem::path path = project->settings.path.parent_path() / (PathManagerPage::get()->get_selected_path_name() + ".csv");

  std::cout << "Exporting path to " << path << std::endl;

  std::ofstream file(path);
  file.clear();

  file << "time,x_pos,y_pos,velocity,rotation,action\n";
  for (std::size_t i = 0; i < cached_curve_points.size(); i-=-1) {
      file << cached_times.at(i) << ','
      << cached_curve_points.at(i).x << ',' << cached_curve_points.at(i).y << ','
      << cached_velocities.at(i) << ',' << cached_rotations.at(i) << ','
      << cached_action_flags.at(i) << '\n';
  }
}

void PathEditorPage::present_curve_editor() {
  const ImGuiStyle& style(ImGui::GetStyle());
  const ImGuiIO& io(ImGui::GetIO());
  ImDrawList* draw_list(ImGui::GetWindowDrawList());
  ImGuiWindow* win(ImGui::GetCurrentWindow());
  if (win->SkipItems) return;

  ImGui::SetScrollX(0);
  ImGui::SetScrollY(0);

  // --- Setup the canvas ---

  // Fit the canvas to the window.
  ImVec2 win_size(ImGui::GetWindowSize());

  float dim_x(win_size.x),
        dim_y(win_size.y);

  // Fit within the window size while maintaining aspect ratio.
  if ((dim_x / dim_y) > field_aspect_ratio) {
    dim_x = dim_y * field_aspect_ratio;
  }
  else {
    dim_y = dim_x / field_aspect_ratio;
  }

  ImVec2 canvas(dim_x, dim_y);

  bb = ImRect(win->DC.CursorPos, win->DC.CursorPos + canvas);

  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0)) return;

  // --- Panning and Zooming ---

  if (focused) {
    // Panning.
    static ImVec2 last_mouse_pos(0.0f, 0.0f);
    ImVec2 mouse_pos(io.MousePos);

    if (ImGui::IsMouseDragging(2)) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
      ImVec2 delta(to_field_coord(mouse_pos, false) - to_field_coord(last_mouse_pos, false));

      field_offset.x += delta.x / field_scale;
      field_offset.y += delta.y / field_scale;
    }

    last_mouse_pos = io.MousePos;

    static float last_field_scale = 1.0f;

    // Zooming.
    ImVec2 mouse_pos_pre_zoom(to_field_coord(mouse_pos));

    field_scale += io.MouseWheel * 0.05f;
    if (field_scale < 1.0f) {
      field_scale = 1.0f;
    }
    else if (field_scale > 4.0f) {
      field_scale = 4.0f;
    }

    ImVec2 mouse_pos_post_zoom(to_field_coord(mouse_pos));

    if (last_field_scale != field_scale) {
      float dx = (mouse_pos_post_zoom.x - mouse_pos_pre_zoom.x);
      float dy = (mouse_pos_post_zoom.y - mouse_pos_pre_zoom.y);
      
      field_offset.x += dx;
      field_offset.y += dy;
    }

    last_field_scale = field_scale;
  }

  // --- Draw the field image ---

  ImVec2 img_offset(field_offset);
  {
    img_offset.x /= FIELD_X;
    img_offset.y /= FIELD_Y;

    img_offset.x *= (project->settings.field.max.x - project->settings.field.min.x);
    img_offset.y *= (project->settings.field.max.y - project->settings.field.min.y);

    img_offset = ImVec2(img_offset.x, img_offset.y) * (bb.Max - bb.Min);
  }

  ImVec2 img_min = ImVec2(bb.Min.x + img_offset.x, bb.Min.y - img_offset.y);
  ImVec2 img_max = ImVec2(bb.Max.x + img_offset.x, bb.Max.y - img_offset.y);

  img_min *= field_scale;
  img_max *= field_scale;

  draw_list->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(field_tex)), img_min, img_max);

  // --- Curve tooltip ---

  decltype(cached_curve_points)::const_iterator it(find_curve_point(io.MousePos.x, io.MousePos.y));
  if (it != cached_curve_points.cend()) {
    ImGui::SetTooltip(ICON_FA_CLOCK "  %f s\n" ICON_FA_RUNNING "  %f m/s", cached_times.at(it - cached_curve_points.cbegin()), cached_velocities.at(it - cached_curve_points.cbegin()));
  }

  // --- Point movement ---

  if (focused) {
    ImVec2 mouse(io.MousePos);
    auto move_point = [&](float& x, float& y) {
      ImVec2 pt(to_field_coord(mouse));
      x = pt.x;
      y = pt.y;

      updated = true;

      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    };

    auto move_curve_point = [&](CurvePointTable::iterator it) {
      float x = it->px,
            y = it->py;

      float prev_x(x),
            prev_y(y);

      move_point(x, y);

      if (x != prev_x || y != prev_y) {
        it->translate(x - prev_x, y - prev_y);
      }

      ImGui::SetTooltip("%.2f m, %.2f m", x, y);
    };

    auto move_rot_point = [&](CurvePointTable::iterator it) {
      auto& rot = it->rotation;

      auto [ax, ay] = it->get_rot_pt();
      float prev_x(ax),
            prev_y(ay);

      move_point(ax, ay);

      if (ax != prev_x || ay != prev_y) {
        it->set_rot_pt(ax, ay);
      }

      if (rot < 0.0f) {
        rot += M_PI * 2.0f;
      }

      ImGui::SetTooltip("%.2f°", rot * RAD_2_DEG);
    };

    auto move_tangent_point = [&](CurvePointTable::iterator it, bool first) {
      auto& head = first ? it->h0 : it->h1;

      auto [x0, y0] = *it->get_tangent_pt(first);

      float prev_x(x0),
            prev_y(y0);

      move_point(x0, y0);

      if (x0 != prev_x || y0 != prev_y) {
        it->set_tangent_pt(first, x0, y0);
      }
      
      if (head < 0.0f) {
        head += M_PI * 2.0f;

        if (!it->stop) {
          (first ? it->h1 : it->h0) = head;
        }
      }

      float tooltip = head - M_PI * (!first && (it->stop || it->end));
      if (tooltip < 0.0f) {
        tooltip += M_PI * 2.0f;
      }
      ImGui::SetTooltip("%.2f°", tooltip * RAD_2_DEG);
    };

    static CurvePointTable::iterator drag_pt = PathManagerPage::get()->get_selected_path().end();
    enum { DRAG_NONE = 0, DRAG_PT = 1 << 0, DRAG_TAN_0 = 1 << 1, DRAG_TAN_1 = 1 << 2, DRAG_ANG = 1 << 3 };
    static std::size_t drag_pt_type(DRAG_NONE);

    if (drag_pt != PathManagerPage::get()->get_selected_path().end() && drag_pt_type != DRAG_NONE && (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))) {
      if (drag_pt_type & DRAG_PT) {
        move_curve_point(drag_pt);
      }
      else if (drag_pt_type & DRAG_ANG) {
        move_rot_point(drag_pt);
      }
      else {
        move_tangent_point(drag_pt, drag_pt_type & DRAG_TAN_0);
      }
    }
    else {
      // Reset the selection.
      drag_pt = PathManagerPage::get()->get_selected_path().end();
      drag_pt_type = DRAG_NONE;

      for (CurvePointTable::iterator it = PathManagerPage::get()->get_selected_path().begin(); it != PathManagerPage::get()->get_selected_path().end(); ++it) {
        auto& x = it->px,
            & y = it->py;

        // Checks the distance from the mouse to a point.
        auto check_dist = [&](float px, float py) -> bool {
          ImVec2 pos(to_draw_coord(ImVec2(px, py)));
          float dist(std::hypotf(pos.x - mouse.x, pos.y - mouse.y));
          return dist < POINT_RADIUS * 2.0f;
        };

        std::optional<ImVec2> c0 = it->get_tangent_pt(true);
        std::optional<ImVec2> c1 = it->get_tangent_pt(false);
        auto [ax, ay] = it->get_rot_pt();

        bool p_dist = check_dist(x, y),
             c0_dist = c0 ? check_dist(c0->x, c0->y) : false,
             c1_dist = c1 ? check_dist(c1->x, c1->y) : false,
             a_dist = check_dist(ax, ay);

        if (p_dist || c0_dist || c1_dist || a_dist) {
          ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0)) {
          drag_pt_type |= p_dist * DRAG_PT;
          if (show_tangents) {
            drag_pt_type |= c0_dist * DRAG_TAN_0;
            drag_pt_type |= c1_dist * DRAG_TAN_1;
          }
          if (show_rotation) {
            drag_pt_type |= a_dist * DRAG_ANG;
          }

          // A point is selected.
          if (drag_pt_type != DRAG_NONE) {
            drag_pt = it;
            selected_pt = it;
            break;
          }
          else if (focused) {
            // Check if the mouse is within the bounding box.
            if (mouse.x >= bb.Min.x && mouse.x <= bb.Max.x && mouse.y >= bb.Min.y && mouse.y <= bb.Max.y) {
              selected_pt = PathManagerPage::get()->get_selected_path().end();
            }
          }
        }
      }
    }

    if (ImGui::IsMouseDoubleClicked(0)) {
      std::pair<CurvePointTable::const_iterator, float> closest_point(PathManagerPage::get()->get_selected_path().end(), std::numeric_limits<float>::max());

      auto get_dist = [&](float x, float y) -> float {
        // Checks the distance from the mouse to a point.
        ImVec2 pos(to_draw_coord(ImVec2(x, y)));
        return std::sqrt(std::powf(pos.x - mouse.x, 2) + std::powf(pos.y - mouse.y, 2));
      };

      // Find the closest point to the mouse.
      for (CurvePointTable::const_iterator it = PathManagerPage::get()->get_selected_path().cbegin(); it != PathManagerPage::get()->get_selected_path().cend(); ++it) {
        auto x = it->px,
             y = it->py;

        // Checks the distance from the mouse to a point.
        float dist(get_dist(x, y));
        
        if (dist < closest_point.second) {
          closest_point = std::make_pair(it, dist);
        }
      }

      ImVec2 new_pt = to_field_coord(mouse);

      auto& [pt, dist] = closest_point;

      bool pt_added(false);
      if (pt == PathManagerPage::get()->get_selected_path().cbegin()) {
        float x_mid((pt->px + (pt + 1)->px) / 2),
              y_mid((pt->py + (pt + 1)->py) / 2);

        if (get_dist(x_mid, y_mid) > dist) {
          // Insert the point at the start of the list.
          selected_pt = PathManagerPage::get()->get_selected_path().insert(PathManagerPage::get()->get_selected_path().cbegin(), { new_pt.x, new_pt.y, M_PI_2, M_PI_2, 1.0f, 1.0f, 0.0f, false, true, false, 0 });
          (selected_pt + 1)->begin = false;
          updated = true;
          pt_added = true;
        }
      }

      if (!pt_added) {
        auto [p, t] = find_curve_part_point(new_pt.x, new_pt.y);
        if (p != PathManagerPage::get()->get_selected_path().cend()) {
          ImVec2 p0(calc_curve_point(p, t));
          ImVec2 p1(calc_curve_point(p, t + INTEGRAL_PRECISION));

          float dx(p1.x - p0.x),
                dy(p1.y - p0.y);

          float angle(std::atan2(dy, dx));

          selected_pt = PathManagerPage::get()->get_selected_path().insert(p + 1, { new_pt.x, new_pt.y, angle, angle, 1.0f, 1.0f, 0.0f, false, false, false, 0 });
          updated = true;
          pt_added = true;
        }
      }

      if (!pt_added) {
        // To the end of the list!
        selected_pt = PathManagerPage::get()->get_selected_path().insert(PathManagerPage::get()->get_selected_path().cend(), { new_pt.x, new_pt.y, M_PI_2, M_PI_2, 1.0f, 1.0f, 0.0f, false, false, true, 0 });
        (selected_pt - 1)->end = false;
        updated = true;
      }
    }
  }

  // --- Drawing ---

  if (updated) {
    cache_values();
  }

  // Draw lines connecting the points of the spline.
  for (std::vector<ImVec2>::const_iterator it = cached_curve_points.cbegin(); it != cached_curve_points.cend(); ++it) {
    if (it + 1 == cached_curve_points.cend()) break;
    
    std::size_t i(it - cached_curve_points.cbegin());

    auto my_clamp = [](float value, float min, float max) -> float {
        if (value > max) {
            value = max;
        }
        else if (value < min) {
            value = min;
        }
        return value;
    };

    float hue;
    switch (curve_style) {
      case CurveStyle::VELOCITY:
        // Dark blue is low velocity, bright red is high velocity.
        hue = 0.8f - (cached_velocities.at(i) / project->settings.max_vel);
        break;
      case CurveStyle::CURVATURE:
        // Dark blue is low curvature, bright red is high curvature.
        hue = 0.6f - (my_clamp(cached_curvatures.at(i), 0.0f, 50.0f) / 50.0f);
        break;
    }

    ImVec2 p0(*it),
           p1(*(it + 1));

    draw_list->AddLine(to_draw_coord(p0), to_draw_coord(p1), ImColor::HSV(hue, 1.0f, 1.0f), CURVE_THICKNESS);
  }

  // Draw the curve waypoints and tangent lines.
  for (CurvePointTable::const_iterator it(PathManagerPage::get()->get_selected_path().cbegin()); it != PathManagerPage::get()->get_selected_path().cend(); ++it) {
    auto x = it->px,
         y = it->py;

    std::optional<ImVec2> c0 = it->get_tangent_pt(true);
    std::optional<ImVec2> c1 = it->get_tangent_pt(false);
    auto [ax, ay] = it->get_rot_pt();

    ImVec2 p(to_draw_coord(ImVec2(x, y)));
    ImVec2 r(to_draw_coord(ImVec2(ax, ay)));

    ImColor pt_color,
            border_color(ImColor(style.Colors[ImGuiCol_Text]));

    if (it == selected_pt) {
      pt_color = ImColor(252, 186, 3, 255);
      border_color = pt_color;
    }
    else if (it == PathManagerPage::get()->get_selected_path().cbegin()) {
      // Green
      pt_color = ImColor(0, 255, 0, 255);
    }
    else if (it == PathManagerPage::get()->get_selected_path().cend() - 1) {
      // Red
      pt_color = ImColor(255, 0, 0, 255);
    }
    else {
      pt_color = ImColor(style.Colors[ImGuiCol_Text]);
    }

    if (show_tangents) {
      if (c0) {
        draw_list->AddLine(p, to_draw_coord(*c0), ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
        draw_list->AddCircle(to_draw_coord(*c0), POINT_RADIUS, ImColor(252, 186, 3, 255), 0, POINT_BORDER_THICKNESS);
      }
      if (c1) {
        draw_list->AddLine(p, to_draw_coord(*c1), ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
        draw_list->AddCircle(to_draw_coord(*c1), POINT_RADIUS, ImColor(252, 186, 3, 255), 0, POINT_BORDER_THICKNESS);
      }
    }

    draw_list->AddCircleFilled(p, POINT_RADIUS, pt_color);

    if (show_rotation) {
      draw_list->AddCircleFilled(r, POINT_RADIUS, border_color);

      ImVec2 rot_pts[4];
      for (std::size_t i = 0; i < 4; ++i) {
        rot_pts[i] = to_draw_coord(it->get_rot_corner_pt(i));
      }
      draw_list->AddQuad(rot_pts[1], rot_pts[0], rot_pts[2], rot_pts[3], border_color);
    }
  }
  updated = false;
}

std::vector<ImVec2> PathEditorPage::calc_curve_points() const {
  std::vector<ImVec2> res;

  for (CurvePointTable::const_iterator it = PathManagerPage::get()->get_selected_path().cbegin(); it + 1 != PathManagerPage::get()->get_selected_path().cend(); ++it) {
    std::size_t i(it - PathManagerPage::get()->get_selected_path().cbegin());
    float len(cached_curve_lengths.at(i));

    std::size_t samples(len * CURVE_RESOLUTION_FACTOR);

    for (std::size_t j = 0; j < samples; ++j) {
      float t(j / static_cast<float>(samples));
      res.push_back(calc_curve_point(it, t));
    }
  }

  return res;
}

ImVec2 PathEditorPage::calc_curve_point(CurvePointTable::const_iterator it, float t) const {
  // The points and the slopes of the tangents.
  float px0(it->px),
        py0(it->py),
        px1((it + 1)->px),
        py1((it + 1)->py);

  // The interpolated point on the spline.

  std::optional<ImVec2> c0 = it->get_tangent_pt(true),
                        c1 = (it + 1)->get_tangent_pt(false);
  float c0x = 0.0f, c0y = 0.0f,
        c1x = 0.0f, c1y = 0.0f;

  if (c0) {
    c0x = c0->x;
    c0y = c0->y;
  }
  if (c1) {
    c1x = c1->x;
    c1y = c1->y;
  }

  // (1 - t)^3
  auto b0 = [](float t) -> float {
    return std::powf(1 - t, 3);
  };

  // 3(1 - t)^2t
  auto b1 = [](float t) -> float {
    return 3 * std::powf(1 - t, 2) * t;
  };

  // 3(1 - t)t^2
  auto b2 = [](float t) -> float {
    return 3 * (1 - t) * std::powf(t, 2);
  };

  // t^3
  auto b3 = [](float t) -> float {
    return std::powf(t, 3);
  };

  // Bezier curve equation.
  float pt_y((py0 * b0(t)) + CTRL_POINT_IMPACT * (c0y * b1(t)) + CTRL_POINT_IMPACT * (c1y * b2(t)) + (py1 * b3(t)));
  float pt_x((px0 * b0(t)) + CTRL_POINT_IMPACT * (c0x * b1(t)) + CTRL_POINT_IMPACT * (c1x * b2(t)) + (px1 * b3(t)));

  return ImVec2(pt_x, pt_y);
}

std::vector<float> PathEditorPage::calc_curve_lengths() const {
  std::vector<float> lengths;

  for (CurvePointTable::const_iterator it(PathManagerPage::get()->get_selected_path().cbegin()); it + 1 != PathManagerPage::get()->get_selected_path().cend(); ++it) {
    lengths.push_back(calc_curve_part_length(it));
  }

  return lengths;
}

float PathEditorPage::calc_curve_part_length(CurvePointTable::const_iterator it) const {
  float len(0.0f);
  
  for (float t(0.0f); t < 1.0f - INTEGRAL_PRECISION; t += INTEGRAL_PRECISION) {
    ImVec2 p0(calc_curve_point(it, t)),
           p1(calc_curve_point(it, t + INTEGRAL_PRECISION));

    float dy(p1.y - p0.y),
          dx(p1.x - p0.x);

    // My friend pythagoras.
    len += std::hypotf(dy, dx);
  }

  return len;
}

std::vector<float> PathEditorPage::calc_curvature() const {
  std::vector<float> curvatures;

  for (decltype(cached_curve_points)::const_iterator it(cached_curve_points.cbegin()); it != cached_curve_points.cend(); ++it) {
    if (it == cached_curve_points.cbegin() || it == cached_curve_points.cend() - 1) {
      curvatures.push_back(0.0f);
      continue;
    }

    ImVec2 p0(*(it - 1)),
           p1(*it),
           p2(*(it + 1));

    float dx0(p1.x - p2.x),
          dy0(p1.y - p2.y),
          dx1(p0.x - p2.x),
          dy1(p0.y - p2.y),
          dx2(p0.x - p1.x),
          dy2(p0.y - p1.y);

    // Side lengths.
    float a(std::hypotf(dx0, dy0)),
          b(std::hypotf(dx1, dy1)),
          c(std::hypotf(dx2, dy2));

    // Semi-perimeter.
    float s((a + b + c) / 2.0f);

    // Heron's formula.
    float A(std::sqrtf(s * (s - a) * (s - b) * (s - c)));
    
    // Menger's curvature formula.
    float curv((4 * A) / (a * b * c));

    // Idk why it happens, but it does.
    if (std::isnan(curv)) {
      curv = 0.0f;
    }

    curvatures.push_back(curv);
  }

  return curvatures;
}

struct PathInterval {
  std::size_t start,
              end;

  enum class Type {
    REGULAR,
    CLAMPED,
  };

  Type type;

  PathInterval(std::size_t start, std::size_t end, Type type)
  : start(start), end(end), type(type) { }
};

std::tuple<std::vector<float>, std::vector<float>, std::vector<float>, std::vector<unsigned>> PathEditorPage::calc_velocity_time() const {
  std::vector<std::pair<float, float>> clamped_intervals;

  {
    float d_traveled(0.0f);

    float start(0.0f);
    bool in_clamped_interval(false);

    // Find intervals in which the curvature is greater than the threshold.
    for (decltype(cached_curvatures)::const_iterator it(cached_curvatures.cbegin()); it != cached_curvatures.cend(); ++it) {
      if (it != cached_curvatures.cend() - 1 && *it >= CURVATURE_THRESHOLD) {
        if (!in_clamped_interval) {
          // The beginning of the interval.
          start = d_traveled;
          in_clamped_interval = true;
        }
      }
      else if (in_clamped_interval) {
        // The end of the interval.
        in_clamped_interval = false;
        clamped_intervals.emplace_back(start, d_traveled);
      }

      if (it != cached_curvatures.cbegin()) {
        std::size_t i(it - cached_curvatures.cbegin());
        float dx((cached_curve_points.at(i).x - cached_curve_points.at(i - 1).x)),
              dy((cached_curve_points.at(i).y - cached_curve_points.at(i - 1).y));

        d_traveled += std::hypotf(dx, dy);
      }
    }
  }

  std::vector<float> velocities, times, rotations;
  std::vector<unsigned> action_flags;

  {
    std::vector<float> stop_point_dists;
    std::vector<float> point_dists;

    // Calculate the total distance of the path.
    float d_total(0.0f);
    for (decltype(cached_curve_points)::const_iterator it(cached_curve_points.cbegin()); it != cached_curve_points.cend(); ++it) {
      if (it == cached_curve_points.cbegin()) continue;

      float dx((it->x - (it - 1)->x)),
            dy((it->y - (it - 1)->y));

      d_total += std::hypotf(dx, dy);

      for (auto pt_it(PathManagerPage::get()->get_selected_path().cbegin()); pt_it != PathManagerPage::get()->get_selected_path().cend(); ++pt_it) {
        if (pt_it->px == it->x && pt_it->py == it->y) {
          point_dists.push_back(d_total);
          if (pt_it->stop) {
            stop_point_dists.push_back(d_total);
          }
          break;
        }
      }
    }

    // Include the last point in the path.
    stop_point_dists.push_back(d_total);
    point_dists.push_back(d_total);

    std::cout << "point dists " << point_dists.size() << '\n';
    for (int i = 0; i < point_dists.size(); ++i) {
        std::cout << "dist " << point_dists.at(i) << '\n';
    }

    decltype(stop_point_dists)::const_iterator stop_it(stop_point_dists.cbegin());
    decltype(point_dists)::const_iterator rot_it(point_dists.cbegin()),
                                          pt_it(point_dists.cbegin());

    float d_traveled(0.0f);
    float t_elapsed(0.0f);

    for (decltype(cached_curve_points)::const_iterator it(cached_curve_points.cbegin()); it != cached_curve_points.cend(); ++it) {
      if (it == cached_curve_points.cbegin()) {
        velocities.push_back(0.0f);
        times.push_back(0.0f);
        rotations.push_back(PathManagerPage::get()->get_selected_path().front().rotation);
        action_flags.push_back(PathManagerPage::get()->get_selected_path().front().actions);
        continue;
      }

      const float last_vel((it == cached_curve_points.cbegin()) ? 0.0f : velocities.back());

      // Calculate the change in distance.
      float d_delta(0.0f);
      float dx(it->x - (it - 1)->x),
            dy(it->y - (it - 1)->y);
      
      d_delta = std::hypotf(dx, dy);

      d_traveled += d_delta;

      if (d_traveled > *stop_it) {
        ++stop_it;
      }
      if (d_traveled > *rot_it && rot_it != point_dists.cend() - 1) {
        ++rot_it;
      }
      if (d_traveled >= *pt_it && pt_it != point_dists.cend() - 1) {
        action_flags.push_back(PathManagerPage::get()->get_selected_path().at(pt_it - point_dists.cbegin() - 1).actions);
        ++pt_it;
      }
      else if (it == cached_curve_points.cend() - 1) {
          action_flags.push_back(PathManagerPage::get()->get_selected_path().back().actions);
      }
      else {
        action_flags.push_back(0);
      }

      // The next target rotation of the robot at this point.
      rotations.push_back(PathManagerPage::get()->get_selected_path().at(rot_it - point_dists.cbegin() + 1).rotation);

      // Distance required to decelerate to a stop.
      float d_to_stop((-std::powf(last_vel, 2.0f)) / (2.0f * -project->settings.max_accel));
      if (d_traveled >= *stop_it - d_to_stop) {
        // Decelerate to a stop.
        float vel(std::sqrtf(std::powf(last_vel, 2.0f) + (2.0f * -project->settings.max_accel * d_delta)));
        if (std::isnan(vel)) vel = 0.0f;
        velocities.push_back(vel);
        t_elapsed += (vel - last_vel) / (-project->settings.max_accel);
      }
      else {
        bool in_clamped_interval(false);
        decltype(clamped_intervals)::const_iterator clamped_interval_it(clamped_intervals.cend());

        for (decltype(clamped_intervals)::const_iterator it(clamped_intervals.cbegin()); it != clamped_intervals.cend(); ++it) {
          if (it->first <= d_traveled && it->second > d_traveled) {
            in_clamped_interval = true;
            clamped_interval_it = it;
            break;
          }
          if (it->second <= d_traveled && it != clamped_intervals.cend() - 1 && (it + 1)->first > d_traveled) {
            clamped_interval_it = it + 1;
            break;
          }
          else if (it == clamped_intervals.cbegin() && it->first > d_traveled) {
            clamped_interval_it = it;
            break;
          }
        }

        if (in_clamped_interval) {
          if (last_vel >= SLOW_CRUISE_VELOCITY) {
            // Cruise at slow speed.
            velocities.push_back(SLOW_CRUISE_VELOCITY);
            t_elapsed += d_delta / SLOW_CRUISE_VELOCITY;
          }
          else {
            // Accelerate to slow speed.
            float vel(std::sqrtf(std::powf(last_vel, 2.0f) + 2.0f * project->settings.max_accel * d_delta));
            if (std::isnan(vel)) vel = 0.0f;

            velocities.push_back(vel);
            t_elapsed += (vel - last_vel) / project->settings.max_accel;
          }
        }
        else {
          float d_to_slow_cruise((std::powf(SLOW_CRUISE_VELOCITY, 2.0f) - std::powf(last_vel, 2.0f)) / (2.0f * project->settings.max_accel * (last_vel < SLOW_CRUISE_VELOCITY ? +1 : -1)));

          if (clamped_interval_it != clamped_intervals.cend() && d_traveled >= clamped_interval_it->first - d_to_slow_cruise) {
            // Accelerate / decelerate to slow speed.
            float vel(std::sqrtf(std::powf(last_vel, 2.0f) + 2.0f * project->settings.max_accel * (last_vel < SLOW_CRUISE_VELOCITY ? +1 : -1) * d_delta));

            velocities.push_back(vel);
            t_elapsed += (vel - last_vel) / (project->settings.max_accel * (last_vel < SLOW_CRUISE_VELOCITY ? +1 : -1));
          }
          else
          if (last_vel >= project->settings.max_vel) {
            // Cruise at max speed.
            velocities.push_back(project->settings.max_vel);
            t_elapsed += d_delta / project->settings.max_vel;
          }
          else {
            // Accelerate to max speed.
            float vel(std::sqrtf(std::powf(last_vel, 2.0f) + 2.0f * project->settings.max_accel * d_delta));

            velocities.push_back(vel);
            t_elapsed += (vel - last_vel) / project->settings.max_accel;
          }
        }
      }
      times.push_back(t_elapsed);

      if (it == cached_curve_points.cend() - 1) {
        velocities.back() = 0.0f;
        continue;
      }
    }
  }

  return std::make_tuple(velocities, times, rotations, action_flags);
}

std::pair<PathEditorPage::CurvePointTable::const_iterator, float> PathEditorPage::find_curve_part_point(float x, float y) const {
  for (CurvePointTable::const_iterator it(PathManagerPage::get()->get_selected_path().cbegin()); it + 1 != PathManagerPage::get()->get_selected_path().cend(); ++it) {
    std::size_t i(it - PathManagerPage::get()->get_selected_path().cbegin());
    float len(cached_curve_lengths.at(i));

    std::size_t samples(len * 64.0f);
    for (std::size_t j(0); j < samples; ++j) {
      float t(j / static_cast<float>(samples));
      ImVec2 p(calc_curve_point(it, t));

      if (std::hypotf(p.x - x, p.y - y) < 0.2f) {
        return std::make_pair(it, t);
      }
    }
  }
  return std::make_pair(PathManagerPage::get()->get_selected_path().cend(), 0.0f);
}

std::vector<ImVec2>::const_iterator PathEditorPage::find_curve_point(float x, float y) const {
  return std::find_if(cached_curve_points.cbegin(), cached_curve_points.cend(), [&](const ImVec2& pt) {
    ImVec2 new_pt(to_draw_coord(pt));
    return std::hypotf(x - new_pt.x, y - new_pt.y) < POINT_RADIUS;
  });
}

void PathEditorPage::cache_values() {
  cached_curve_lengths = calc_curve_lengths();
  cached_curve_points = calc_curve_points();
  cached_curvatures = calc_curvature();
  auto [vels, times, rotations, action_flags] = calc_velocity_time();
  cached_velocities = std::move(vels);
  cached_times = std::move(times);
  cached_rotations = std::move(rotations);
  cached_action_flags = std::move(action_flags);
}

PathEditorPage PathEditorPage::instance {};
