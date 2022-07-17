#include <pages/path_editor.h>
#include <imgui_internal.h>
#include <cmath>
#include <utility>
#include <vector>
#include <iostream>
#include <limits>

#define CURVE_RESOLUTION_FACTOR 128.0f
#define CURVE_THICKNESS 2
#define TANGENT_THICKNESS 1
#define POINT_RADIUS 5
#define POINT_BORDER_THICKNESS 2
#define INTEGRAL_PRECISION 0.0001f

#define ROT_POINT_RADIUS 0.05f
#define ROBOT_WIDTH 0.04f

PathEditorPage::PathEditorPage() { }

PathEditorPage::~PathEditorPage() { }

void PathEditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Editor", running,
                ImGuiWindowFlags_NoCollapse
              | ImGuiWindowFlags_UnsavedDocument * unsaved)) {
    ImGui::End();
    return;
  }

  focused = ImGui::IsWindowFocused();

  present_curve_editor();
  
  ImGui::End();
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

  float dim_x = win_size.x * 0.98f;
  float dim_y = win_size.y * 0.9f;

  if (dim_x < dim_y) {
    dim_y = dim_x;
  } else {
    dim_x = dim_y;
  }

  ImVec2 canvas(dim_x, dim_y);

  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + canvas);
  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0)) return;

  const ImGuiID id = win->GetID("Path Editor");

  ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 1), true, style.FrameRounding);

  // --- Point movement ---

  ImVec2 mouse = io.MousePos;
  auto move_point = [&](float& x, float& y) {
    x = mouse.x;
    y = mouse.y;

    x = (x - bb.Min.x) / (bb.Max.x - bb.Min.x);
    y = 1 - (y - bb.Min.y) / (bb.Max.y - bb.Min.y);
    updated = true;
  };

  auto move_curve_point = [&](CurvePointTable::iterator it) {
    auto& [x, y, c0x, c0y, c1x, c1y, ax, ay] = *it;

    float prev_x = x, prev_y = y;

    move_point(x, y);

    if (x != prev_x || y != prev_y) {
      float dx = x - prev_x,
            dy = y - prev_y;
      // Translate the tangent points with the point.
      c0x += dx;
      c0y += dy;
      c1x += dx;
      c1y += dy;
      ax += dx;
      ay += dy;
    }

    ImGui::SetTooltip("%.2f, %.2f", x, y);
  };

  auto move_rot_point = [&](CurvePointTable::iterator it) {
    auto& [x, y, c0x, c0y, c1x, c1y, ax, ay] = *it;

    float prev_x = ax, prev_y = ay;

    move_point(ax, ay);
    if (ax != prev_x || ay != prev_y) {
      float ang = std::atan2(y - ay, x - ax);
      ax = x + std::cos(ang) * -ROT_POINT_RADIUS;
      ay = y + std::sin(ang) * -ROT_POINT_RADIUS;
    }

    ImGui::SetTooltip("%.2f, %.2f", ax, ay);
  };

  auto move_tangent_point = [&](CurvePointTable::iterator it, bool first) {
    auto& [_x, _y, c0x, c0y, c1x, c1y, ax, ay] = *it;

    float& x0 = first ? c0x : c1x,
         & y0 = first ? c0y : c1y,
         & x1 = first ? c1x : c0x,
         & y1 = first ? c1y : c0y;

    float prev_x = x0, prev_y = y0;
    move_point(x0, y0);

    if (x0 != prev_x || y0 != prev_y) {
      // The angle of the moved tangent point.
      float angle = std::atan2(y0 - _y, x0 - _x) + 3.14159265358979323846f;
      
      // The distance of the other tangent point from the point on the curve.
      float prev_dist = std::sqrt(std::powf(x1 - _x, 2) + std::powf(y1 - _y, 2));
      
      // The new position of the other tangent point (180 degrees rotated from
      // the moved tangent point with its distance to the curve preserved).
      x1 = _x + std::cos(angle) * prev_dist;
      y1 = _y + std::sin(angle) * prev_dist;
    }

    ImGui::SetTooltip("%.2f, %.2f", x0, y0);
  };

  static CurvePointTable::iterator selected = points.end();
  enum { SELECT_NONE = 0, SELECT_PT = 1 << 0, SELECT_TAN_0 = 1 << 1, SELECT_TAN_1 = 1 << 2, SELECT_ANG = 1 << 3 };
  static std::size_t select_mode = SELECT_NONE;

  if (selected != points.end() && select_mode != SELECT_NONE && (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))) {
    if (select_mode & SELECT_PT) {
      move_curve_point(selected);
    }
    else if (select_mode & SELECT_ANG) {
      move_rot_point(selected);
    }
    else {
      move_tangent_point(selected, select_mode & SELECT_TAN_0);
    }
  }
  else {
    // Reset the selection.
    selected = points.end();
    select_mode = SELECT_NONE;

    if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0)) {
      for (CurvePointTable::iterator it = points.begin(); it != points.end(); ++it) {
        auto& [x, y, c0x, c0y, c1x, c1y, ax, ay] = *it;

        // Checks the from the mouse to a point.
        auto check_dist = [&](float px, float py) -> bool {
          ImVec2 pos = ImVec2(px, 1 - py) * (bb.Max - bb.Min) + bb.Min;
          float dist = std::sqrt(std::powf(pos.x - mouse.x, 2) + std::powf(pos.y - mouse.y, 2));
          return dist < POINT_RADIUS * 4;
        };

        select_mode |= check_dist(x, y) * SELECT_PT;
        select_mode |= check_dist(c0x, c0y) * SELECT_TAN_0;
        select_mode |= check_dist(c1x, c1y) * SELECT_TAN_1;
        select_mode |= check_dist(ax, ay) * SELECT_ANG;

        // A point is selected.
        if (select_mode != SELECT_NONE) {
          selected = it;
          break;
        }
      }
    }
  }

  if (ImGui::IsMouseDoubleClicked(0)) {
    std::pair<CurvePointTable::const_iterator, float> closest_point(points.end(), std::numeric_limits<float>::max());

    auto get_dist = [&](float x, float y) -> float {
      // Checks the distance from the mouse to a point.
      ImVec2 pos = ImVec2(x, 1 - y) * (bb.Max - bb.Min) + bb.Min;
      return std::sqrt(std::powf(pos.x - mouse.x, 2) + std::powf(pos.y - mouse.y, 2));
    };

    // Find the closest point to the mouse.
    for (CurvePointTable::const_iterator it = points.cbegin(); it != points.cend(); ++it) {
      const auto [x, y, c0x, c0y, c1x, c1y, ax, ay] = *it;

      // Checks the distance from the mouse to a point.
      float dist = get_dist(x, y);
      
      if (dist < closest_point.second) {
        closest_point = std::make_pair(it, dist);
      }
    }

    ImVec2 new_pt((mouse.x - bb.Min.x) / (bb.Max.x - bb.Min.x), 1 - (mouse.y - bb.Min.y) / (bb.Max.y - bb.Min.y));

    bool dir = true;
    auto& [pt, dist] = closest_point;

    float neighbor_dist = std::numeric_limits<float>::max();

    bool pt_added = false;
    if (pt == points.cbegin()) {
      float x_mid = (pt->px + (pt + 1)->px) / 2,
            y_mid = (pt->py + (pt + 1)->py) / 2;

      if (get_dist(x_mid, y_mid) > dist) {
        // Insert the point at the start of the list.
        points.insert(points.cbegin(), { new_pt.x, new_pt.y, new_pt.x, new_pt.y + 0.1f, new_pt.x, new_pt.y - 0.1f, new_pt.x, new_pt.y + ROT_POINT_RADIUS });
        updated = true;
        pt_added = true;
      }
    }

    if (!pt_added) {
      auto [p, t] = find_curve_point(new_pt.x, new_pt.y);
      if (p != points.cend()) {
        ImVec2 p0 = calc_curve_point(p, t);
        ImVec2 p1 = calc_curve_point(p, t + INTEGRAL_PRECISION);

        float dx = p1.x - p0.x,
              dy = p1.y - p0.y;

        float angle = std::atan2(dy, dx);

        float ang_cos = std::cos(angle),
              ang_sin = std::sin(angle);

        float ext = 0.1f;

        points.insert(p + 1, { new_pt.x, new_pt.y, new_pt.x + ang_cos * ext, new_pt.y + ang_sin * ext, new_pt.x - ang_cos * ext, new_pt.y - ang_sin * ext, new_pt.x, new_pt.y + ROT_POINT_RADIUS });
        updated = true;
        pt_added = true;
      }
    }

    if (!pt_added) {
      // To the end of the list!
      points.insert(points.cend(), { new_pt.x, new_pt.y, new_pt.x, new_pt.y + 0.1f, new_pt.x, new_pt.y - 0.1f, new_pt.x, new_pt.y + ROT_POINT_RADIUS });
      updated = true;
    }
  }

  // --- Drawing ---

  if (updated) {
    cached_curve_lengths = calc_curve_lengths();
    cached_curve_points = calc_curve_points();
    cached_curvatures = calc_curvature();
  }

  static float highest_curve = 0;
  // Draw lines connecting the points of the spline.
  for (std::vector<ImVec2>::const_iterator it = cached_curve_points.cbegin(); it != cached_curve_points.cend(); ++it) {
    if (it + 1 == cached_curve_points.cend()) break;
    
    std::size_t i = it - cached_curve_points.cbegin();

    // Blue is low curvature, red is high curvature.
    float hue = 0.6f - (std::clamp(cached_curvatures.at(i), 0.0f, 50.0f) / 50.0f);

    ImVec2 p0 = *it, p1 = *(it + 1);

    draw_list->AddLine(ImVec2(p0.x, 1 - p0.y) * (bb.Max - bb.Min) + bb.Min, ImVec2(p1.x, 1 - p1.y) * (bb.Max - bb.Min) + bb.Min, ImColor::HSV(hue, 1.0f, 1.0f), CURVE_THICKNESS);
  }

  if (show_handles) {
    // Draw the curve waypoints and tangent lines.
    for (const auto& [x, y, c0x, c0y, c1x, c1y, ax, ay] : points) {
      ImVec2 p = ImVec2(x, 1 - y) * (bb.Max - bb.Min) + bb.Min;
      ImVec2 c0 = ImVec2(c0x, 1 - c0y) * (bb.Max - bb.Min) + bb.Min;
      ImVec2 c1 = ImVec2(c1x, 1 - c1y) * (bb.Max - bb.Min) + bb.Min;
      ImVec2 r = ImVec2(ax, 1 - ay) * (bb.Max - bb.Min) + bb.Min;

      draw_list->AddLine(p, c0, ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
      draw_list->AddLine(p, c1, ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
      draw_list->AddCircleFilled(p, POINT_RADIUS, ImColor(style.Colors[ImGuiCol_Text]));
      draw_list->AddCircle(c0, POINT_RADIUS, ImColor(252, 186, 3, 255), 0, POINT_BORDER_THICKNESS);
      draw_list->AddCircle(c1, POINT_RADIUS, ImColor(252, 186, 3, 255), 0, POINT_BORDER_THICKNESS);
      draw_list->AddCircleFilled(r, POINT_RADIUS, ImColor(style.Colors[ImGuiCol_Text]));

      // Draw the robot's rotation.
      ImVec2 fr, fl, br, bl;
      {
        float angle = std::atan2(y - ay, x - ax);
        float ax1 = x + std::cos(angle) * ROT_POINT_RADIUS,
              ay1 = y + std::sin(angle) * ROT_POINT_RADIUS;
        fr = ImVec2(ax + std::cos(angle - M_PI_2) * ROBOT_WIDTH, 1.0f - (ay + std::sin(angle - M_PI_2) * ROBOT_WIDTH)) * (bb.Max - bb.Min) + bb.Min;
        fl = ImVec2(ax + std::cos(angle + M_PI_2) * ROBOT_WIDTH, 1.0f - (ay + std::sin(angle + M_PI_2) * ROBOT_WIDTH)) * (bb.Max - bb.Min) + bb.Min;
        br = ImVec2(ax1 + std::cos(angle - M_PI_2) * ROBOT_WIDTH, 1.0f - (ay1 + std::sin(angle - M_PI_2) * ROBOT_WIDTH)) * (bb.Max - bb.Min) + bb.Min;
        bl = ImVec2(ax1 + std::cos(angle + M_PI_2) * ROBOT_WIDTH, 1.0f - (ay1 + std::sin(angle + M_PI_2) * ROBOT_WIDTH)) * (bb.Max - bb.Min) + bb.Min;
      }
      draw_list->AddQuad(fr, fl, bl, br, ImColor(style.Colors[ImGuiCol_Text]));
    }
  }
  updated = false;
}

std::vector<ImVec2> PathEditorPage::calc_curve_points() const {
  std::vector<ImVec2> res;

  for (CurvePointTable::const_iterator it = points.cbegin(); it + 1 != points.cend(); ++it) {
    std::size_t i = it - points.cbegin();
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
    float m0 = (it->c0y - py0) / (it->c0x - px0),
          m1 = ((it + 1)->c0y - py1) / ((it + 1)->c0x - px1);

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
  } else {
    float c0x = it->c0x,
          c0y = it->c0y,
          c1x = (it + 1)->c1x,
          c1y = (it + 1)->c1y;

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

  for (CurvePointTable::const_iterator it = points.cbegin(); it + 1 != points.cend(); ++it) {
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
    float a = std::sqrtf(dx0 * dx0 + dy0 * dy0),
          b = std::sqrtf(dx1 * dx1 + dy1 * dy1),
          c = std::sqrtf(dx2 * dx2 + dy2 * dy2);

    // Semi-perimeter.
    float s = (a + b + c) / 2.0f;

    // Heron's formula.
    float A = std::sqrtf(s * (s - a) * (s - b) * (s - c));
    
    // Menger's curvature formula.
    float curv = (4 * A) / (a * b * c);

    curvatures.push_back(curv);
  }

  return curvatures;
}

std::pair<PathEditorPage::CurvePointTable::const_iterator, float> PathEditorPage::find_curve_point(float x, float y) const {
  for (CurvePointTable::const_iterator it = points.cbegin(); it + 1 != points.cend(); ++it) {
    std::size_t i = it - points.cbegin();
    float len = cached_curve_lengths.at(i);

    std::size_t samples = len * 64.0f;
    for (std::size_t j = 0; j < samples; ++j) {
      float t = j / static_cast<float>(samples);
      ImVec2 p = calc_curve_point(it, t);

      if (std::sqrtf((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y)) < 0.02f) {
        return std::make_pair(it, t);
      }
    }
  }
  return std::make_pair(points.cend(), 0.0f);
}

PathEditorPage PathEditorPage::instance {};
