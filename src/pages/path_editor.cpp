#include <pages/path_editor.h>
#include <imgui_internal.h>
#include <cmath>
#include <utility>
#include <vector>
#include <iostream>

#define CURVE_SAMPLES 64.0f
#define CURVE_THICKNESS 2
#define TANGENT_THICKNESS 1
#define POINT_RADIUS 5
#define POINT_BORDER_THICKNESS 2

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
  ImGui::Text("%d\n", focused);

  present_spline_editor();
  
  ImGui::End();
}

void PathEditorPage::present_spline_editor() {
  const ImGuiStyle& style = ImGui::GetStyle();
  const ImGuiIO& io = ImGui::GetIO();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems) return;

  // Setup the canvas.
  const float dim = 500;
  ImVec2 canvas(dim, dim);

  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + canvas);
  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0)) return;

  const ImGuiID id = win->GetID("Path Editor");

  ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 1), true, style.FrameRounding);

  // Draw the background grid.
  for (int i = 0; i <= canvas.x; i += (canvas.x / 16)) {
    draw_list->AddLine(
      ImVec2(bb.Min.x + i, bb.Min.y),
      ImVec2(bb.Min.x + i, bb.Max.y),
      ImGui::GetColorU32(ImGuiCol_TextDisabled));
  }
  for (int i = 0; i <= canvas.y; i += (canvas.y / 16)) {
    draw_list->AddLine(
      ImVec2(bb.Min.x, bb.Min.y + i),
      ImVec2(bb.Max.x, bb.Min.y + i),
      ImGui::GetColorU32(ImGuiCol_TextDisabled));
  }

  bool pt_selected = false;

  auto move_point = [&](float& x, float& y) {
    ImVec2 pos = ImVec2(x, 1- y) * (bb.Max - bb.Min) + bb.Min;
    ImVec2 mouse = io.MousePos;
    float dist = std::sqrt(std::powf(pos.x - mouse.x, 2) + std::powf(pos.y - mouse.y, 2));
    if (dist < POINT_RADIUS * 2) {
      ImGui::SetTooltip("(%.2f, %.2f)", x, y);

      if (!pt_selected && (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))) {
        pt_selected = true;
        pos.x = mouse.x;
        pos.y = mouse.y;

        x = (pos.x - bb.Min.x) / (bb.Max.x - bb.Min.x);
        y = 1 - (pos.y - bb.Min.y) / (bb.Max.y - bb.Min.y);
      }
    }
  };

  ImVec2 mouse = io.MousePos;
  for (auto& [x, y, mx, my] : points) {
    float prev_x = x, prev_y = y;
      move_point(x, y);
      // Move the tangent point along with the point.
      if (x != prev_x || y != prev_y) {
        mx += x - prev_x;
        my += y - prev_y;
      }
      else {
        move_point(mx, my);
      }
  }

  // Draw points and tangents.
  for (auto& [x, y, mx, my] : points) {
    ImVec2 p = ImVec2(x, 1 - y) * (bb.Max - bb.Min) + bb.Min;
    ImVec2 m = ImVec2(mx, 1- my) * (bb.Max - bb.Min) + bb.Min;

    draw_list->AddLine(p, m, ImColor(235, 64, 52, 255), TANGENT_THICKNESS);
    draw_list->AddCircleFilled(p, POINT_RADIUS, ImColor(style.Colors[ImGuiCol_Text]));
    draw_list->AddCircleFilled(m, POINT_RADIUS, ImColor(252, 186, 3, 255));
  }

  // Calculate points of the spline.
  std::vector<ImVec2> line_points = calc_cubic_hermite_spline_point();

  // Draw lines connecting the points of the spline.
  for (std::vector<ImVec2>::const_iterator it = line_points.cbegin(); it != line_points.cend(); ++it) {
    if (it + 1 == line_points.cend()) break;

    ImVec2 p0 = *it, p1 = *(it + 1);

    draw_list->AddLine(ImVec2(p0.x, 1 - p0.y) * (bb.Max - bb.Min) + bb.Min, ImVec2(p1.x, 1 - p1.y) * (bb.Max - bb.Min) + bb.Min, ImColor(235, 64, 52, 255), CURVE_THICKNESS);
  }
}

std::vector<ImVec2> PathEditorPage::calc_cubic_hermite_spline_point() const {
  std::vector<ImVec2> res;
  for (std::size_t i = 0; i < CURVE_SAMPLES; ++i) {
    // 0-1 x value.
    float x = i / CURVE_SAMPLES;

    std::vector<SplinePointTable::const_iterator> pt_iters;
    for (SplinePointTable::const_iterator it = points.cbegin(); it != points.cend(); ++it) {
      if (it->x >= x && (it == points.begin() || (it - 1)->x < x)) {
        pt_iters.push_back(it - 1);
      }
    }

    if (pt_iters.empty()) continue;

    for (SplinePointTable::const_iterator it : pt_iters) {
      // The points and the tangents.
      float px0 = it->x,
            py0 = it->y,
            m0 = it->my * 10,
            px1 = (it + 1)->x,
            py1 = (it + 1)->y,
            m1 = (it + 1)->my * 10;

      float t = (x - px0) / (px1 - px0);

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

      // The interpolated point on the spline.

      // p(t) = h00(t)pk + h10(t)(xk+1 - xk)mk + h01(t)pk+1 + h11(t)(xk+1 - xk)mk+1
      float y = (py0 * h00(t)) + (py1 * h01(t)) + (m0 * h10(t) * (px1 - px0)) + (m1 * h11(t) * (px1 - px0));

      if (y > 0.01f) {
        res.push_back(ImVec2(x, y));
      }
    }
  }

  return res;
}

PathEditorPage PathEditorPage::instance {};
