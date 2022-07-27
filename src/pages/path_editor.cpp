#include <pages/path_editor.h>
#include <project.h>
#include <stb_image.h>
#include <glad/glad.h>

#define CURVE_RESOLUTION_FACTOR 128.0f
#define CURVE_THICKNESS 2
#define TANGENT_THICKNESS 1
#define POINT_RADIUS 5
#define POINT_BORDER_THICKNESS 2
#define INTEGRAL_PRECISION 0.0001f

#define ROT_POINT_RADIUS 25.0f
#define ROBOT_WIDTH 20.0f

ImVec2 PathEditorPage::CurvePoint::get_tangent_pt(bool first) const {
  ImVec2 pt = to_draw_coord(ImVec2(px, py));

  pt.x = pt.x + std::cos(heading + (M_PI * !first)) * (first ? w0 : w1);
  pt.y = pt.y + std::sin(heading + (M_PI * !first)) * (first ? w0 : w1);

  pt = to_field_coord(pt);

  return pt;
}

void PathEditorPage::CurvePoint::set_tangent_pt(bool first, float x, float y) {
  ImVec2 pt0 = to_draw_coord(ImVec2(x, y)),
         pt1 = to_draw_coord(ImVec2(px, py));

  heading = std::atan2(pt0.y - pt1.y, pt0.x - pt1.x) + (M_PI * !first);

  float& w = first ? w0 : w1;

  w = std::hypotf(pt0.x - pt1.x, pt0.y - pt1.y);
}

ImVec2 PathEditorPage::CurvePoint::get_rot_pt(bool reverse) const {
  float v = reverse * 2 - 1;

  ImVec2 pt = to_draw_coord(ImVec2(px, py));

  pt.x = pt.x + std::cos(rotation + M_PI * v) * ROT_POINT_RADIUS * v;
  pt.y = pt.y + std::sin(rotation + M_PI * v) * ROT_POINT_RADIUS * v;

  pt = to_field_coord(pt);

  return pt;
}

// 0 = FL, 1 = FR, 2 = BL, 3 = BR
ImVec2 PathEditorPage::CurvePoint::get_rot_corner_pt(int index) const {
  bool reverse = false;
  if (index > 1) {
    reverse = true;
    index -= 2;
  }

  ImVec2 pt = to_draw_coord(get_rot_pt(reverse));

  pt.x = pt.x + std::cos(rotation + M_PI_2 * ((2 * index) - 1)) * ROBOT_WIDTH;
  pt.y = pt.y + std::sin(rotation + M_PI_2 * ((2 * index) - 1)) * ROBOT_WIDTH;

  pt = to_field_coord(pt);
  return pt;
}

void PathEditorPage::CurvePoint::set_rot_pt(float x, float y) {
  // rotation = std::atan2(y - py, x - px);

  ImVec2 pt0 = to_draw_coord(ImVec2(x, y)),
         pt1 = to_draw_coord(ImVec2(px, py));

  rotation = std::atan2(pt0.y - pt1.y, pt0.x - pt1.x);
}

void PathEditorPage::CurvePoint::translate(float dx, float dy) {
  px += dx;
  py += dy;
}

PathEditorPage::PathEditorPage() { }

PathEditorPage::~PathEditorPage() { }

ImVec2 PathEditorPage::to_field_coord(ImVec2 pt) {
  pt = ImVec2((pt.x - bb.Min.x) / (bb.Max.x - bb.Min.x), 1 - (pt.y - bb.Min.y) / (bb.Max.y - bb.Min.y));
  return un_adjust_field_coord(pt);
}

ImVec2 PathEditorPage::to_draw_coord(ImVec2 pt) {
  pt = adjust_field_coord(pt);
  return ImVec2(pt.x, 1 - pt.y) * (bb.Max - bb.Min) + bb.Min;
}

ImVec2 PathEditorPage::adjust_field_coord(ImVec2 pt) {
  const Field& field = project->settings.field;

  pt.x *= (field.max.x - field.min.x);
  pt.y *= (field.max.y - field.min.y);

  pt.x += field.min.x;
  pt.y += field.min.y;

  return pt;
}

ImVec2 PathEditorPage::un_adjust_field_coord(ImVec2 pt) {
  const Field& field = project->settings.field;

  pt.x -= field.min.x;
  pt.y -= field.min.y;

  pt.x /= (field.max.x - field.min.x);
  pt.y /= (field.max.y - field.min.y);

  return pt;
}

std::optional<PathEditorPage::CurvePointTable::iterator> PathEditorPage::get_selected_point() {
  if (selected_pt == project->points.end()) {
    return std::nullopt;
  }
  return selected_pt;
}

void PathEditorPage::delete_point() {
    if (selected_pt != project->points.end() && project->points.size() > 1) {
        project->points.erase(selected_pt);
        selected_pt = project->points.end();
        updated = true;
    }
}

void PathEditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Editor", running,
                ImGuiWindowFlags_NoCollapse
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

void PathEditorPage::set_project(Project* _project) {
  project = _project;
  selected_pt = project->points.end();

  int width, height, nr_channels;
  unsigned char* img_data = stbi_load(project->settings.field.img_path.c_str(), &width, &height, &nr_channels, 0);
  if (img_data) {
    int tex_channels = nr_channels == 3 ? GL_RGB : GL_RGBA;

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

  updated = true;
}

void PathEditorPage::export_path(std::string path) {
  cached_curve_lengths = calc_curve_lengths();
  cached_curve_points = calc_curve_points();
  cached_curvatures = calc_curvature();
  auto [vels, times] = calc_velocity_time();
  cached_velocities = vels;
  cached_times = times;

  auto replace_macro = [&](std::string macro, std::string value) {
    std::size_t pos;
    while (pos = path.find("${" + macro + "}"), pos != std::string::npos) {
      path.replace(pos, macro.length() + 3, value);
    }
  };

  replace_macro("PROJECT_DIR", project->settings.path.parent_path().string());
  replace_macro("PATH_NAME", "the_path");

  std::cout << "Exporting path to " << path << std::endl;

  std::ofstream file(path);

  file << "time, x_pos, y_pos, velocity\n";
  for (std::size_t i = 0; i < cached_curve_points.size(); i-=-1) {
      file << cached_times.at(i) << ','
      << (cached_curve_points.at(i).x * FIELD_X) << ',' << (cached_curve_points.at(i).y * FIELD_Y) << ','
      << cached_velocities.at(i) << '\n';
  }
}

void PathEditorPage::present_curve_editor() {
  const ImGuiStyle& style = ImGui::GetStyle();
  const ImGuiIO& io = ImGui::GetIO();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems) return;

  // Setup the canvas.

  // Fit the canvas to the window.
  ImVec2 win_size = ImGui::GetWindowSize();

  float dim_x = win_size.x,
        dim_y = win_size.y;

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

  const ImGuiID id = win->GetID("Path Editor");

  draw_list->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(field_tex)), bb.Min, bb.Max);

  // --- Point movement ---

  if (focused) {
    ImVec2 mouse = io.MousePos;
    auto move_point = [&](float& x, float& y) {
      ImVec2 pt = to_field_coord(mouse);
      x = pt.x;
      y = pt.y;

      updated = true;
    };

    auto move_curve_point = [&](CurvePointTable::iterator it) {
      auto [x, y, head, w0, w1, rot] = *it;

      float prev_x = x, prev_y = y;

      move_point(x, y);

      if (x != prev_x || y != prev_y) {
        it->translate(x - prev_x, y - prev_y);
      }

      ImGui::SetTooltip("%.2f, %.2f", x * FIELD_X, y * FIELD_Y);
    };

    auto move_rot_point = [&](CurvePointTable::iterator it) {
      auto& [x, y, head, w0, w1, rot] = *it;

      auto [ax, ay] = it->get_rot_pt();
      float prev_x = ax, prev_y = ay;

      move_point(ax, ay);

      if (ax != prev_x || ay != prev_y) {
        it->set_rot_pt(ax, ay);
      }

      if (rot < 0.0f) {
        rot += M_PI * 2.0f;
      }

      ImGui::SetTooltip("%.2f degrees", rot * RAD_2_DEG);
    };

    auto move_tangent_point = [&](CurvePointTable::iterator it, bool first) {
      auto& [x, y, head, w0, w1, rot] = *it;

      auto [x0, y0] = it->get_tangent_pt(first);

      float prev_x = x0, prev_y = y0;
      move_point(x0, y0);

      if (x0 != prev_x || y0 != prev_y) {
        it->set_tangent_pt(first, x0, y0);
      }
      
      if (head < 0.0f) {
        head += M_PI * 2.0f;
      }

      ImGui::SetTooltip("%.2f degrees", head * RAD_2_DEG);
    };

    static CurvePointTable::iterator drag_pt = project->points.end();
    enum { DRAG_NONE = 0, DRAG_PT = 1 << 0, DRAG_TAN_0 = 1 << 1, DRAG_TAN_1 = 1 << 2, DRAG_ANG = 1 << 3 };
    static std::size_t drag_pt_type = DRAG_NONE;

    if (drag_pt != project->points.end() && drag_pt_type != DRAG_NONE && (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))) {
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
      drag_pt = project->points.end();
      drag_pt_type = DRAG_NONE;

      if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0)) {
        for (CurvePointTable::iterator it = project->points.begin(); it != project->points.end(); ++it) {
          auto& [x, y, head, w0, w1, rot] = *it;

          // Checks the from the mouse to a point.
          auto check_dist = [&](float px, float py) -> bool {
            ImVec2 pos = to_draw_coord(ImVec2(px, py));
            float dist = std::hypotf(pos.x - mouse.x, pos.y - mouse.y);
            return dist < POINT_RADIUS * 4;
          };

          auto [c0x, c0y] = it->get_tangent_pt(true);
          auto [c1x, c1y] = it->get_tangent_pt(false);
          auto [ax, ay] = it->get_rot_pt();

          drag_pt_type |= check_dist(x, y) * DRAG_PT;
          drag_pt_type |= check_dist(c0x, c0y) * DRAG_TAN_0;
          drag_pt_type |= check_dist(c1x, c1y) * DRAG_TAN_1;
          drag_pt_type |= check_dist(ax, ay) * DRAG_ANG;

          // A point is selected.
          if (drag_pt_type != DRAG_NONE) {
            drag_pt = it;
            selected_pt = it;
            break;
          }
          else if (focused) {
            // Check if the mouse is within the bounding box.
            if (mouse.x >= bb.Min.x && mouse.x <= bb.Max.x && mouse.y >= bb.Min.y && mouse.y <= bb.Max.y) {
              selected_pt = project->points.end();
            }
          }
        }
      }
    }

    if (ImGui::IsMouseDoubleClicked(0)) {
      std::pair<CurvePointTable::const_iterator, float> closest_point(project->points.end(), std::numeric_limits<float>::max());

      auto get_dist = [&](float x, float y) -> float {
        // Checks the distance from the mouse to a point.
        ImVec2 pos = to_draw_coord(ImVec2(x, y));
        return std::sqrt(std::powf(pos.x - mouse.x, 2) + std::powf(pos.y - mouse.y, 2));
      };

      // Find the closest point to the mouse.
      for (CurvePointTable::const_iterator it = project->points.cbegin(); it != project->points.cend(); ++it) {
        const auto [x, y, head, w0, w1, rot] = *it;

        // Checks the distance from the mouse to a point.
        float dist = get_dist(x, y);
        
        if (dist < closest_point.second) {
          closest_point = std::make_pair(it, dist);
        }
      }

      ImVec2 new_pt = to_field_coord(mouse);

      bool dir = true;
      auto& [pt, dist] = closest_point;

      float neighbor_dist = std::numeric_limits<float>::max();

      bool pt_added = false;
      if (pt == project->points.cbegin()) {
        float x_mid = (pt->px + (pt + 1)->px) / 2,
              y_mid = (pt->py + (pt + 1)->py) / 2;

        if (get_dist(x_mid, y_mid) > dist) {
          // Insert the point at the start of the list.
          selected_pt = project->points.insert(project->points.cbegin(), { new_pt.x, new_pt.y, M_PI_2, 50.0f, 50.0f, 0.0f });
          updated = true;
          pt_added = true;
        }
      }

      if (!pt_added) {
        auto [p, t] = find_curve_point(new_pt.x, new_pt.y);
        if (p != project->points.cend()) {
          ImVec2 p0 = calc_curve_point(p, t);
          ImVec2 p1 = calc_curve_point(p, t + INTEGRAL_PRECISION);

          float dx = p1.x - p0.x,
                dy = p1.y - p0.y;

          float angle = std::atan2(dy, dx);

          selected_pt = project->points.insert(p + 1, { new_pt.x, new_pt.y, angle, 50.0f, 50.0f, 0.0f });
          updated = true;
          pt_added = true;
        }
      }

      if (!pt_added) {
        // To the end of the list!
        selected_pt = project->points.insert(project->points.cend(), { new_pt.x, new_pt.y, M_PI_2, 50.0f, 50.0f, 0.0f });
        updated = true;
      }
    }
  }

  ImVec2 mouse_pt = to_field_coord(io.MousePos);

  std::vector<ImVec2>::const_iterator it = std::find_if(cached_curve_points.cbegin(), cached_curve_points.cend(), [&](const ImVec2& pt) {
    ImVec2 new_pt = to_draw_coord(pt);
    return std::hypotf(io.MousePos.x - new_pt.x, io.MousePos.y - new_pt.y) < POINT_RADIUS;
  });

  if (it != cached_curve_points.cend()) {
    ImGui::SetTooltip("%f m/s, %f s", cached_velocities.at(it - cached_curve_points.cbegin()), cached_times.at(it - cached_curve_points.cbegin()));
  }


  // --- Drawing ---

  if (updated) {
    cached_curve_lengths = calc_curve_lengths();
    cached_curve_points = calc_curve_points();
    cached_curvatures = calc_curvature();
    auto [vels, times] = calc_velocity_time();
    cached_velocities = vels;
    cached_times = times;
  }

  static float highest_curve = 0;
  // Draw lines connecting the points of the spline.
  for (std::vector<ImVec2>::const_iterator it = cached_curve_points.cbegin(); it != cached_curve_points.cend(); ++it) {
    if (it + 1 == cached_curve_points.cend()) break;
    
    std::size_t i = it - cached_curve_points.cbegin();

    auto my_clamp = [](float value, float min, float max) -> float {
        if (value > max) {
            value = max;
        }
        else if (value < min) {
            value = min;
        }
        return value;
    };

    // Blue is low curvature, red is high curvature.
    // float hue = 0.6f - (my_clamp(cached_curvatures.at(i), 0.0f, 50.0f) / 50.0f);
    float hue = 0.8f - (cached_velocities.at(i) / project->settings.max_vel);

    ImVec2 p0 = *it, p1 = *(it + 1);

    draw_list->AddLine(to_draw_coord(p0), to_draw_coord(p1), ImColor::HSV(hue, 1.0f, 1.0f), CURVE_THICKNESS);
  }

  // Draw the curve waypoints and tangent lines.
  for (CurvePointTable::const_iterator it = project->points.cbegin(); it != project->points.cend(); ++it) {
    const auto& [x, y, head, w0, w1, rot] = *it;

    auto [c0x, c0y] = it->get_tangent_pt(true);
    auto [c1x, c1y] = it->get_tangent_pt(false);
    auto [ax, ay] = it->get_rot_pt();

    ImVec2 p = to_draw_coord(ImVec2(x, y));
    ImVec2 c0 = to_draw_coord(ImVec2(c0x, c0y));
    ImVec2 c1 = to_draw_coord(ImVec2(c1x, c1y));
    ImVec2 r = to_draw_coord(ImVec2(ax, ay));

    ImColor pt_color, border_color = ImColor(style.Colors[ImGuiCol_Text]);

    if (it == selected_pt) {
      pt_color = ImColor(252, 186, 3, 255);
      border_color = pt_color;
    }
    else if (it == project->points.cbegin()) {
      // Green
      pt_color = ImColor(0, 255, 0, 255);
    }
    else if (it == project->points.cend() - 1) {
      // Red
      pt_color = ImColor(255, 0, 0, 255);
    }
    else {
      pt_color = ImColor(style.Colors[ImGuiCol_Text]);
    }

    if (show_tangents) {
      draw_list->AddLine(p, c0, ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
      draw_list->AddLine(p, c1, ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
      draw_list->AddCircle(c0, POINT_RADIUS, ImColor(252, 186, 3, 255), 0, POINT_BORDER_THICKNESS);
      draw_list->AddCircle(c1, POINT_RADIUS, ImColor(252, 186, 3, 255), 0, POINT_BORDER_THICKNESS);
    }
    draw_list->AddCircleFilled(p, POINT_RADIUS, pt_color);
    draw_list->AddCircleFilled(r, POINT_RADIUS, border_color);

    // Draw the robot's rotation.
    ImVec2 fl, fr, bl, br;
    {
      auto [ax1, ay1] = it->get_rot_pt(true);
      
      fl = to_draw_coord(it->get_rot_corner_pt(0));
      fr = to_draw_coord(it->get_rot_corner_pt(1));
      bl = to_draw_coord(it->get_rot_corner_pt(2));
      br = to_draw_coord(it->get_rot_corner_pt(3));
    }
    draw_list->AddQuad(fr, fl, bl, br, border_color);
  }
  updated = false;
}

std::vector<ImVec2> PathEditorPage::calc_curve_points() const {
  std::vector<ImVec2> res;

  for (CurvePointTable::const_iterator it = project->points.cbegin(); it + 1 != project->points.cend(); ++it) {
    std::size_t i = it - project->points.cbegin();
    float len = cached_curve_lengths.at(i);

    std::size_t samples = len * CURVE_RESOLUTION_FACTOR;

    for (std::size_t j = 0; j < samples; ++j) {
      float t = j / static_cast<float>(samples);
      res.push_back(calc_curve_point(it, t));
    }
  }

  return res;
}

ImVec2 PathEditorPage::calc_curve_point(CurvePointTable::const_iterator it, float t) const {
  // The points and the slopes of the tangents.
  float px0 = it->px,
        py0 = it->py,
        px1 = (it + 1)->px,
        py1 = (it + 1)->py;

  // The interpolated point on the spline.

  // Cubic hermite spline.
  if (curve_kind == CurveKind::CUBIC_HERMITE) {
      auto [c0x, c0y] = it->get_tangent_pt(true);
      auto [c1x, c1y] = (it + 1)->get_tangent_pt(true);

      float m0 = (c0y - py0) / (c0x - px0),
            m1 = (c1y - py1) / (c1x - px1);

    // The hermite base functions.

    // h00(t) = 2t^3 - 3t^2 + 1
    auto h00 = [](float t) -> float {
      return 2.0f * std::powf(t, 3) - 3.0f * std::powf(t, 2) + 1.0f;
    };
    // h10(t) = t^3 - 2t^2 + t
    auto h10 = [](float t) -> float {
      return std::powf(t, 3) - 2.0f * (std::powf(t, 2)) + t;
    };
    // h01(t) = -2t^3 + 3t^2
    auto h01 = [](float t) -> float {
      return -2.0f * std::powf(t, 3) + 3.0f * std::powf(t, 2);
    };
    // h11(t) = t^3 - t^2
    auto h11 = [](float t) -> float {
      return std::powf(t, 3) - std::powf(t, 2);
    };

    // p(t) = h00(t)pk + h10(t)(xk+1 - xk)mk + h01(t)pk+1 + h11(t)(xk+1 - xk)mk+1
    float pt_y = (py0 * h00(t)) + (py1 * h01(t)) + (m0 * h10(t) * (px1 - px0)) + (m1 * h11(t) * (px1 - px0));
    float pt_x = (t * (px1 - px0)) + px0;

    if (pt_y > 0.01f && pt_y < 0.99f) {
      return ImVec2(pt_x, pt_y);
    }
  // Cubic bezier curve.
  }
  else {
      auto [c0x, c0y] = it->get_tangent_pt(true);
      auto [c1x, c1y] = (it + 1)->get_tangent_pt(false);

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

    float pt_y = (py0 * b0(t)) + (c0y * b1(t)) + (c1y * b2(t)) + (py1 * b3(t));
    float pt_x = (px0 * b0(t)) + (c0x * b1(t)) + (c1x * b2(t)) + (px1 * b3(t));

    return ImVec2(pt_x, pt_y);
  }
  return ImVec2(0, 0);
}

std::vector<float> PathEditorPage::calc_curve_lengths() const {
  std::vector<float> lengths;

  for (CurvePointTable::const_iterator it = project->points.cbegin(); it + 1 != project->points.cend(); ++it) {
    lengths.push_back(calc_curve_part_length(it));
  }

  return lengths;
}

float PathEditorPage::calc_curve_part_length(CurvePointTable::const_iterator it) const {
  float len = 0.0f;
  
  for (float t = 0.0f; t < 1.0f - INTEGRAL_PRECISION; t += INTEGRAL_PRECISION) {
    ImVec2 p0 = calc_curve_point(it, t),
           p1 = calc_curve_point(it, t + INTEGRAL_PRECISION);

    float dy = p1.y - p0.y,
          dx = p1.x - p0.x;

    // My friend pythagoras.
    len += std::sqrtf(dy * dy + dx * dx);
  }

  return len;
}

std::vector<float> PathEditorPage::calc_curvature() const {
  std::vector<float> curvatures;

  for (std::vector<ImVec2>::const_iterator it = cached_curve_points.cbegin(); it != cached_curve_points.cend(); ++it) {
    if (it == cached_curve_points.cbegin() || it == cached_curve_points.cend() - 1) {
      curvatures.push_back(0.0f);
      continue;
    }

    ImVec2 p0 = *(it - 1),
           p1 = *it,
           p2 = *(it + 1);

    float dx0 = p1.x - p2.x,
          dy0 = p1.y - p2.y,
          dx1 = p0.x - p2.x,
          dy1 = p0.y - p2.y,
          dx2 = p0.x - p1.x,
          dy2 = p0.y - p1.y;

    // Side lengths.
    float a = std::hypotf(dx0, dy0),
          b = std::hypotf(dx1, dy1),
          c = std::hypotf(dx2, dy2);

    // Semi-perimeter.
    float s = (a + b + c) / 2.0f;

    // Heron's formula.
    float A = std::sqrtf(s * (s - a) * (s - b) * (s - c));
    
    // Menger's curvature formula.
    float curv = (4 * A) / (a * b * c);

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
};

std::pair<std::vector<float>, std::vector<float>> PathEditorPage::calc_velocity_time() const {
  std::vector<PathInterval> path_intervals;

  // Intervals in which the maximum velocity is adjusted because of a high curvature.
  std::vector<std::pair<std::size_t, std::size_t>> clamped_intervals;

  for (decltype(cached_curvatures)::const_iterator it = cached_curvatures.cbegin(); it != cached_curvatures.cend(); ++it) {
    if (*it > 15.0f) {
      decltype(it) start = it;

      while (++it != cached_curvatures.cend() - 1 && *it > 15.0f);

      clamped_intervals.push_back(std::make_pair(start - cached_curvatures.cbegin(), it - cached_curvatures.cbegin()));
    }
  }

  {
    decltype(clamped_intervals)::const_iterator it = clamped_intervals.cbegin();

    do {
      std::size_t last = it == clamped_intervals.cend() ? cached_curve_points.size() : it->first;

      if (last != 0) {
        std::size_t start = 0;
        if (!path_intervals.empty()) {
          start = path_intervals.back().end;
        }
        path_intervals.push_back({ start, last, PathInterval::Type::REGULAR });
      }

      if (clamped_intervals.empty()) break;

      path_intervals.push_back(PathInterval{ it->first, it->second, PathInterval::Type::CLAMPED });

      if (it == clamped_intervals.cend() - 1 && it->second != cached_curve_points.size() - 1) {
        std::size_t start = it->second;
        if (!path_intervals.empty()) {
          start = path_intervals.back().end;
        }
        path_intervals.push_back(PathInterval{ start, cached_curve_points.size(), PathInterval::Type::REGULAR });
      }

      ++it;
    } while (it != clamped_intervals.cend());
  }

  std::vector<float> velocities, times;

  float t_elapsed = 0.0f;
  for (decltype(path_intervals)::const_iterator it = path_intervals.cbegin(); it != path_intervals.cend(); ++it) {
    auto [start, end, type] = *it;
    
    if (type == PathInterval::Type::REGULAR) {
      // Calculate the length of the path in this interval.
      float d_total = 0.0f;
      for (std::size_t i = start; i < end - (cached_curve_points.size() == end); ++i) {
        // TODO: Calculate velocity :D
        float dx = (cached_curve_points.at(i).x - cached_curve_points.at(i + 1).x) * FIELD_X,
              dy = (cached_curve_points.at(i).y - cached_curve_points.at(i + 1).y) * FIELD_Y;

        d_total += std::hypotf(dx, dy);
      }

      // Get the start velocity and the preferred end velocity.
      float v0 = 0.0f, v1 = 0.0f;

      if (it != path_intervals.cbegin()) {
        v0 = velocities.back();

        if (it != path_intervals.cend() - 1) {
          v1 = 0.3f; // TODO: Change this.
        }
      }

      float v_max = std::sqrtf((2.0f * project->settings.max_accel * d_total + std::powf(v0, 2.0f) + std::powf(v1, 2.0f)) / 2.0f);
      if (v_max > project->settings.max_vel) {
        v_max = project->settings.max_vel;
      }

      // Distance accelerating.
      float d_accel = (std::powf(v_max, 2.0f) - std::powf(v0, 2.0f)) / (2.0f * project->settings.max_accel);
      // Distance decelerating.
      float d_decel = (std::powf(v1, 2.0f) - std::powf(v_max, 2.0f)) / (2.0f * -project->settings.max_accel);

      if (v0 < v1) {
        if (v_max < v1) {
          d_accel = d_total;
          d_decel = 0.0f;
        }
      }
      else if (v_max < v0) {
        d_decel = d_total;
        d_accel = 0.0f;
      }

      // Distance at constant velocity.
      float d_cruise = d_total - d_accel - d_decel;

      float d_travelled = 0.0f;
      for (std::size_t i = start; i < end; ++i) {
        if (d_travelled < d_accel) {
          velocities.push_back(std::sqrtf(std::powf(v0, 2.0f) + 2.0f * project->settings.max_accel * d_travelled));
        }
        else if (d_travelled < d_accel + d_cruise) {
          velocities.push_back(v_max);
        }
        else if (d_travelled < d_accel + d_cruise + d_decel) {
          velocities.push_back(std::sqrtf(std::powf(v_max, 2.0f) + 2.0f * -project->settings.max_accel * (d_travelled - d_accel - d_cruise)));
        }
        else {
          velocities.push_back(v1);
        }

        if (i != end - 1) {
          float dx = (cached_curve_points.at(i).x - cached_curve_points.at(i + 1).x) * FIELD_X,
                dy = (cached_curve_points.at(i).y - cached_curve_points.at(i + 1).y) * FIELD_Y;

          float d = std::hypotf(dx, dy);

          d_travelled += d;

          if (velocities.at(i) > 0.01f) {
            t_elapsed += d / velocities.at(i);
          }

          //std::cout << "time: " << t_elapsed << " dist " << d << " vel " << velocities.at(i) << '\n';
        }

        times.push_back(t_elapsed);
      }
    }
    else {
      for (std::size_t i = start; i < end; ++i) {
        velocities.push_back(0.3f);

        if (i != end - 1) {
          float dx = (cached_curve_points.at(i).x - cached_curve_points.at(i + 1).x) * FIELD_X,
                dy = (cached_curve_points.at(i).y - cached_curve_points.at(i + 1).y) * FIELD_Y;

          float d = std::hypotf(dx, dy);

          if (velocities.at(i) > 0.01f) {
            t_elapsed += d / velocities.at(i);
          }
 
          //std::cout << "time: " << t_elapsed << " dist " << d << " vel " << velocities.at(i) << '\n';
        }

        times.push_back(t_elapsed);
      }
    }
  }
  return std::make_pair(velocities, times);
}

std::pair<PathEditorPage::CurvePointTable::const_iterator, float> PathEditorPage::find_curve_point(float x, float y) const {
  for (CurvePointTable::const_iterator it = project->points.cbegin(); it + 1 != project->points.cend(); ++it) {
    std::size_t i = it - project->points.cbegin();
    float len = cached_curve_lengths.at(i);

    std::size_t samples = len * 64.0f;
    for (std::size_t j = 0; j < samples; ++j) {
      float t = j / static_cast<float>(samples);
      ImVec2 p = calc_curve_point(it, t);

      if (std::hypotf(p.x - x, p.y - y) < 0.02f) {
        return std::make_pair(it, t);
      }
    }
  }
  return std::make_pair(project->points.cend(), 0.0f);
}

PathEditorPage::ExportCurvePointTable PathEditorPage::calc_export_curve_points() const {
  ExportCurvePointTable points;

  float vel = 0.0f;
  for (CurvePointTable::const_iterator it = project->points.cbegin(); it + 1 != project->points.cend(); ++it) {

  }

  return points;
}

PathEditorPage PathEditorPage::instance {};
